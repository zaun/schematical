import CoreGraphics
import Foundation

/// The autorouter's entry point (plan-38-E…J). Nonisolated and
/// deterministic: the same problem gives the same result. It reads only the
/// `RoutingProblem` and never the document.
nonisolated enum Router {
    /// Routes `problem`. `progress` is called after each connection;
    /// `isCancelled` is polled during the search and between connections.
    static func route(_ problem: RoutingProblem, progress: (RoutingProgress) -> Void = { _ in },
                      isCancelled: () -> Bool = { false }) -> RoutingResult {
        RoutingResult()
    }
}
