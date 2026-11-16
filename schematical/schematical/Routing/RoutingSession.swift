import Foundation
import Observation
import os

/// A flag the engine polls from its thread: set once to stop.
nonisolated final class RoutingStopFlag: Sendable {
    private let state = OSAllocatedUnfairLock(initialState: false)
    var isSet: Bool { state.withLock { $0 } }
    func set() { state.withLock { $0 = true } }
}

/// One Autoroute run of a document (plan-38-G § 3): the problem is built on
/// the main actor, the engine runs on a background task, progress comes
/// back to the main actor at most every 0.1 s (with a copper snapshot for a
/// live preview at most every 0.5 s), and the result is applied as one undo
/// step — unless the run was cancelled, or the board changed while it ran,
/// when it is discarded. Stop keeps what is routed so far.
@MainActor
@Observable
final class RoutingSession {
    enum State: Equatable {
        case idle
        case running(RoutingProgress)
        case finished(RoutingSummary)
        case discarded(String)
    }

    static let changedReason = "The board changed while routing"
    static let cancelledReason = "Autoroute was cancelled"

    private(set) var state: State = .idle
    /// The copper routed so far, while running.
    private(set) var preview: (traces: [RoutedTrace], vias: [RoutedVia]) = ([], [])
    /// The last run's problem and result, applied or not.
    private(set) var problem: RoutingProblem?
    private(set) var result: RoutingResult?
    /// Every progress report the session showed, in order (bounded).
    private(set) var reports: [RoutingProgress] = []

    @ObservationIgnored private var flag = RoutingStopFlag()
    @ObservationIgnored private var cancelled = false
    @ObservationIgnored private var task: Task<Void, Never>?
    @ObservationIgnored private var lastReport = Date.distantPast

    /// Nothing here needs the main actor. An isolated deinit, nested in
    /// its owner's (`AutorouteModel`), aborts in the concurrency runtime
    /// (a task-local scope freed twice, `StopLookupScope`).
    nonisolated deinit {}

    var isRunning: Bool {
        if case .running = state { return true }
        return false
    }

    /// Starts routing `document` with `options`. Refused, with the reason in
    /// `editorMessage`, while the simulation runs or a run is under way.
    @discardableResult
    func start(document: SchematicDocument, options: RoutingOptions, rules: DesignRules? = nil) -> Bool {
        let rules = rules ?? document.board.rules
        guard !document.isPlaying else {
            document.editorMessage = "Stop the simulation to autoroute."
            return false
        }
        guard !isRunning else {
            document.editorMessage = "Autoroute is already running."
            return false
        }
        let snapshot = document.state
        let problem = document.routingProblem(options: options, rules: rules)
        self.problem = problem
        result = nil
        reports = []
        preview = ([], [])
        cancelled = false
        let flag = RoutingStopFlag()
        self.flag = flag
        state = .running(RoutingProgress(routed: 0, total: problem.connections, round: 0, net: ""))
        task = Task.detached(priority: .userInitiated) { [weak self] in
            var last = Date.distantPast
            let result = Router.route(problem, progress: { progress in
                let now = Date()
                guard now.timeIntervalSince(last) >= 0.1 || progress.traces != nil else { return }
                last = now
                Task { @MainActor in self?.show(progress) }
            }, isCancelled: { flag.isSet })
            await MainActor.run { self?.finish(result, problem: problem, document: document, snapshot: snapshot) }
        }
        return true
    }

    /// Stops the run; what is routed so far is applied.
    func stop() { flag.set() }

    /// Stops the run and throws its result away.
    func cancel() {
        cancelled = true
        flag.set()
    }

    /// Waits for the run under way to end (tests, and the sheet's close).
    func wait() async {
        await task?.value
    }

    private func show(_ progress: RoutingProgress) {
        guard isRunning else { return }
        var shown = progress
        if let traces = progress.traces {
            preview = (traces, progress.vias ?? [])
            shown.traces = nil
            shown.vias = nil
        }
        state = .running(shown)
        if reports.count < 10_000 { reports.append(shown) }
    }

    private func finish(_ result: RoutingResult, problem: RoutingProblem, document: SchematicDocument,
                        snapshot: DocumentState) {
        self.result = result
        preview = ([], [])
        task = nil
        if cancelled {
            state = .discarded(Self.cancelledReason)
        } else if document.state != snapshot {
            state = .discarded(Self.changedReason)
        } else {
            state = .finished(document.applyRouting(result, problem: problem))
        }
    }
}
