import SwiftUI

/// The Autoroute sheet (plan-38-K): the options, a live summary of what a
/// run would do, the run's progress (Stop keeps what is routed, Cancel
/// keeps nothing), and its results.
struct AutorouteSheet: View {
    @Bindable var model: AutorouteModel
    var onClose: () -> Void
    /// Highlights a net on the canvas (a result row's click).
    var onHighlight: (PlaneNet) -> Void = { _ in }
    var onOpenDesignRuleCheck: () -> Void = {}

    var body: some View {
        VStack(spacing: 0) {
            switch model.session.state {
            case let .running(progress):
                RoutingProgressView(progress: progress, stop: model.stop, cancel: model.cancel)
            default:
                optionsForm
                Divider()
                footer
            }
        }
        .frame(width: 520, height: 640)
        .task(id: SummaryInput(options: model.runOptions, board: NetListInput(model.document))) {
            try? await Task.sleep(for: .milliseconds(150))
            guard !Task.isCancelled else { return }
            model.refreshSummary()
        }
    }

    private struct SummaryInput: Hashable {
        var options: RoutingOptions
        var board: NetListInput
    }

    // MARK: Options

    private var optionsForm: some View {
        Form {
            if case .finished = model.session.state { results }
            if case let .discarded(reason) = model.session.state {
                Section { Text(reason).foregroundStyle(.secondary) }
            }
            Section("Route") {
                Picker("Nets", selection: $model.scopeChoice) {
                    Text("All nets").tag(AutorouteModel.ScopeChoice.all)
                    Text("Selection").tag(AutorouteModel.ScopeChoice.selection)
                }
                Picker("Which", selection: $model.options.include) {
                    Text("Every incomplete net").tag(RoutingOptions.Include.incompleteNets)
                    Text("Only nets with no copper yet").tag(RoutingOptions.Include.untouchedNets)
                }
                if let summary = model.summary {
                    Text("\(summary.nets) \(summary.nets == 1 ? "net" : "nets"), \(summary.connections) "
                         + "\(summary.connections == 1 ? "connection" : "connections") to route")
                        .foregroundStyle(.secondary)
                    ForEach(summary.impossible, id: \.self) { Text($0).foregroundStyle(.orange).font(.callout) }
                }
            }
            Section("Existing copper") {
                Picker("Existing", selection: $model.options.existing) {
                    Text("Keep all").tag(RoutingOptions.Existing.keepAll)
                    Text("Clear autorouted").tag(RoutingOptions.Existing.clearAuto)
                    Text("Clear all unlocked").tag(RoutingOptions.Existing.clearUnlocked)
                }
                if let summary = model.summary, model.options.existing != .keepAll {
                    Text("Removes \(summary.removedTraces) \(summary.removedTraces == 1 ? "trace" : "traces") and "
                         + "\(summary.removedVias) \(summary.removedVias == 1 ? "via" : "vias")")
                        .foregroundStyle(.secondary)
                }
            }
            Section {
                ForEach(model.layerRows) { row in layerRow(row) }
            } header: {
                HStack {
                    Text("Layers")
                    Spacer()
                    Button("Alternate Directions") { model.useAlternatingDirections() }
                        .buttonStyle(.borderless)
                        .help("Top horizontal, Bottom vertical, inner layers in turn")
                }
            }
            Section("Vias") {
                Picker("Vias", selection: $model.options.vias) {
                    Text("Allowed").tag(RoutingOptions.Vias.allowed)
                    Text("None").tag(RoutingOptions.Vias.none)
                }
                .disabled(!model.viasAvailable)
                .help(model.viasAvailable ? "" : "A 1-layer board has no vias")
            }
            Section("Rules") {
                overrideRow("Width", unit: "mil", value: model.options.widthOverride.map(Double.init),
                            standard: Double(NetRules.defaults(for: .current()).width)) { model.options.widthOverride = $0.map { Int($0.rounded()) } }
                overrideRow("Clearance", unit: "mm", value: model.options.clearanceOverride,
                            standard: DesignRules.current().minClearance) { model.options.clearanceOverride = $0 }
                Text("Without an override each net uses its class: \(model.document.board.netClasses.count) "
                     + "\(model.document.board.netClasses.count == 1 ? "class" : "classes") and Default.")
                    .foregroundStyle(.secondary).font(.callout)
            }
            Section("Quality") {
                Picker("Strategy", selection: $model.options.strategy) {
                    Text("Fast").tag(RoutingOptions.Strategy.fast)
                    Text("Balanced").tag(RoutingOptions.Strategy.balanced)
                    Text("Thorough").tag(RoutingOptions.Strategy.thorough)
                }
                Picker("Grid", selection: $model.options.grid) {
                    ForEach(RoutingOptions.grids, id: \.self) { Text(String(format: "%g mm", $0)).tag($0) }
                }
                Picker("Angles", selection: $model.options.angle) {
                    Text("0°, 45° and 90°").tag(RoutingOptions.Angle.octilinear)
                    Text("0° and 90° only").tag(RoutingOptions.Angle.orthogonal)
                }
                overrideRow("Time limit", unit: "s", value: model.options.timeLimit, standard: 60) { model.options.timeLimit = $0 }
            }
            Section("Groups") {
                Toggle("Tune length groups", isOn: $model.options.tuneGroups)
                Toggle("Route pairs side by side", isOn: $model.options.routePairs)
                Toggle("Fan out to Planes", isOn: $model.options.fanoutPlanes)
            }
        }
        .formStyle(.grouped)
    }

    private func layerRow(_ row: AutorouteModel.LayerRow) -> some View {
        HStack {
            Toggle(row.layer.title, isOn: Binding(get: { row.enabled && !row.isPlane },
                                                  set: { model.setLayer(row.layer, enabled: $0) }))
                .disabled(row.isPlane)
            Spacer()
            if row.isPlane {
                Text("Plane (fan-out only)").foregroundStyle(.secondary)
            } else {
                Picker("Direction", selection: Binding(get: { row.direction },
                                                      set: { model.setLayer(row.layer, direction: $0) })) {
                    Text("Any").tag(RoutingOptions.Direction.auto)
                    Text("Horizontal").tag(RoutingOptions.Direction.horizontal)
                    Text("Vertical").tag(RoutingOptions.Direction.vertical)
                }
                .labelsHidden()
                .fixedSize()
            }
        }
    }

    /// A value that is off (nil) until switched on, then a number.
    private func overrideRow(_ title: String, unit: String, value: Double?, standard: Double,
                             set: @escaping (Double?) -> Void) -> some View {
        HStack {
            Toggle(title, isOn: Binding(get: { value != nil }, set: { set($0 ? standard : nil) }))
            Spacer()
            if let value {
                TextField(title, value: Binding(get: { value }, set: { set($0) }), format: .number)
                    .labelsHidden()
                    .multilineTextAlignment(.trailing)
                    .frame(width: 80)
                Text(unit).foregroundStyle(.secondary)
            }
        }
    }

    // MARK: Results

    private var results: some View {
        Section("Result") {
            if let result = model.session.result {
                Text("Routed \(result.stats.routed) of \(result.stats.connections) connections"
                     + (result.stats.stopped ? " (stopped)" : "")
                     + String(format: ", %d vias, %.0f mm, %.1f s", result.stats.vias, result.stats.wireLength,
                              result.stats.seconds))
            }
            ForEach(model.results) { row in
                Button {
                    if let key = row.key { onHighlight(key) }
                } label: {
                    HStack {
                        Text(row.name).fontWeight(.medium)
                        Text("\(row.count) unrouted").foregroundStyle(.red)
                        Spacer()
                        Text(row.reason).foregroundStyle(.secondary).lineLimit(1)
                    }
                }
                .buttonStyle(.plain)
                .help("Highlight \(row.name) on the board")
            }
            ForEach(model.groupLines, id: \.self) { Text($0).foregroundStyle(.secondary) }
            Button("Design Rule Check") { onOpenDesignRuleCheck() }
        }
    }

    private var footer: some View {
        HStack {
            Spacer()
            Button("Close") { onClose() }
                .keyboardShortcut(.cancelAction)
            Button("Route") { model.route() }
                .keyboardShortcut(.defaultAction)
                .disabled(!model.canRoute || (model.summary?.connections ?? 1) == 0)
        }
        .padding()
    }
}

/// A run under way: its round, how much is routed, and Stop / Cancel.
struct RoutingProgressView: View {
    let progress: RoutingProgress
    var stop: () -> Void
    var cancel: () -> Void

    var body: some View {
        VStack(spacing: 16) {
            Spacer()
            Text("Routing…").font(.title2.bold())
            ProgressView(value: Double(progress.routed), total: Double(max(progress.total, 1)))
                .frame(width: 360)
            Text("\(progress.routed) of \(progress.total) connections routed"
                 + (progress.round > 0 ? ", round \(progress.round)" : ""))
            if progress.current < progress.routed {
                Text("\(progress.current) routed right now, while rerouting").foregroundStyle(.secondary)
            }
            if !progress.net.isEmpty { Text(progress.net).foregroundStyle(.secondary).font(.callout) }
            Spacer()
            HStack {
                Button("Cancel", role: .cancel) { cancel() }
                    .keyboardShortcut(.cancelAction)
                    .help("Stop and keep nothing")
                Button("Stop") { stop() }
                    .keyboardShortcut(.defaultAction)
                    .help("Stop and keep what is routed")
            }
            .padding()
        }
        .frame(maxWidth: .infinity)
    }
}
