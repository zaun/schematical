import SwiftUI

/// Step 6 of the Part Wizard, "Footprint" (plan-46-J): pick a library
/// footprint from thumbnails, make one from the numbers on the datasheet's
/// drawing, or place pads one by one.
struct FootprintStepPage: View {
    @Bindable var model: PartEditorModel

    enum Choice: String, CaseIterable {
        case library, make, pads

        var title: String {
            switch self {
            case .library: "Use a library footprint"
            case .make: "Make one from the drawing"
            case .pads: "Place pads one by one"
            }
        }
    }

    @State private var choice = Choice.library
    @State private var search = ""
    @State private var style = ParametricFootprintDraft.Style.dual

    /// What the datasheet's drawing gives, in plain words.
    static let guidance = [
        "Read the recommended land pattern (also called the example board layout or PCB layout) if the "
            + "datasheet has one. It is not the package outline: the pads are larger than the leads.",
        "Where the drawing gives a minimum and a maximum, use the middle: (min + max) / 2.",
        "The drawing is a top view. Pin 1 is at the top of the left side.",
        "The footprint has one pad per pin of the pinout.",
    ]

    var body: some View {
        Section {
            Picker("How", selection: $choice) {
                ForEach(Choice.allCases, id: \.self) { Text($0.title).tag($0) }
            }
            .pickerStyle(.segmented)
            .labelsHidden()
            ForEach(Self.guidance, id: \.self) { line in
                Label(line, systemImage: "ruler").font(.callout).foregroundStyle(.secondary)
            }
        }
        .onAppear { choice = Self.choice(for: model.draft) }

        switch choice {
        case .library: library
        case .make: make
        case .pads: pads
        }

        if model.draft.kind == "socket" {
            FitsEditor(model: model)
        }

        Section("Preview") {
            if let part = model.lastValidPart, part.footprint != nil, model.isValid {
                FootprintPreviewView(part: part, revision: model.previewRevision)
                    .frame(height: 230)
            } else if let message = model.validationMessage {
                Label(message, systemImage: "exclamationmark.triangle").foregroundStyle(.red)
                    .textSelection(.enabled)
            } else {
                Text("The footprint is drawn here once the part loads.").foregroundStyle(.secondary)
            }
        }
    }

    /// The tab a draft opens on: the kind of footprint it already has.
    static func choice(for draft: PartDraft) -> Choice {
        switch draft.inlineFootprint {
        case .explicit: .pads
        case .parametric: .make
        case nil: .library
        }
    }

    // MARK: Library

    @ViewBuilder private var library: some View {
        let pins = model.draft.pinCount
        let candidates = model.validator.footprints.filter { $0.pins == pins }
        let shown = FootprintRanking.rank(FootprintRanking.search(candidates, text: search),
                                          package: model.draft.package)
        Section {
            TextField("Search", text: $search, prompt: Text("SOIC, 1.27 mm, TSSOP …"))
            if pins == 0 {
                Text("Give the part its pins first: the library is searched by pin count.")
                    .foregroundStyle(.secondary)
            } else if shown.isEmpty {
                Text("No library footprint has \(pins) pads\(search.isEmpty ? "" : " and matches the search"). "
                    + "Make one from the drawing instead.")
                    .foregroundStyle(.secondary)
            } else {
                ScrollView {
                    LazyVGrid(columns: [GridItem(.adaptive(minimum: 118), spacing: 8)], spacing: 8) {
                        ForEach(shown, id: \.id) { footprint in
                            Button {
                                model.useLibraryFootprint(footprint.id)
                            } label: {
                                VStack(spacing: 3) {
                                    FootprintThumbnail(footprint: footprint)
                                        .frame(height: 78)
                                    Text(footprint.id).font(.caption).lineLimit(1)
                                }
                                .padding(6)
                                .frame(maxWidth: .infinity)
                                .background(model.draft.footprint == footprint.id
                                    ? Color.accentColor.opacity(0.25) : Color.secondary.opacity(0.08),
                                    in: RoundedRectangle(cornerRadius: 6))
                                .contentShape(Rectangle())
                            }
                            .buttonStyle(.plain)
                            .help(footprint.description)
                            .accessibilityLabel("\(footprint.id): \(footprint.description)")
                        }
                    }
                    .padding(2)
                }
                .frame(height: 330)
            }
        } header: {
            Text("\(shown.count) library \(shown.count == 1 ? "footprint" : "footprints") with \(pins) pads")
        } footer: {
            Text(model.draft.package.isEmpty ? "Pads are drawn to scale; pin 1 is highlighted."
                : "The ones matching the package “\(model.draft.package)” come first. Pin 1 is highlighted.")
        }
        if let id = model.draft.footprint, model.draft.inlineFootprint == nil,
           let chosen = candidates.first(where: { $0.id == id }) {
            Section("Chosen") {
                LabeledContent(chosen.id, value: chosen.description)
                Button("Start from this one") {
                    choice = model.startFromLibraryFootprint(chosen)
                }
                .help("Copy this footprint into the part to change its numbers or pads")
            }
        }
    }

    // MARK: Make one

    private var parametric: Binding<ParametricFootprintDraft>? {
        guard case let .parametric(current)? = model.draft.inlineFootprint else { return nil }
        return Binding(get: {
            if case let .parametric(now)? = model.draft.inlineFootprint { return now }
            return current
        }, set: { model.draft.inlineFootprint = .parametric($0) })
    }

    @ViewBuilder private var make: some View {
        if let footprint = parametric {
            Section {
                Picker("Shape", selection: Binding(
                    get: { footprint.wrappedValue.style },
                    set: { model.startParametricFootprint($0) })) {
                    ForEach(ParametricFootprintDraft.Style.allCases, id: \.self) { style in
                        Text("\(style.title) — \(style.examples)").tag(style)
                    }
                }
                FootprintDiagram(style: footprint.wrappedValue.style)
                if footprint.wrappedValue.pins != model.draft.pinCount {
                    HStack {
                        Label("The footprint has \(footprint.wrappedValue.pins) pads; the pinout has "
                            + "\(model.draft.pinCount) pins.", systemImage: "exclamationmark.circle")
                            .foregroundStyle(.orange)
                        Button("Match the pinout") { footprint.wrappedValue.pins = model.draft.pinCount }
                    }
                }
            } header: {
                Text("The shape")
            }
            Section("The numbers (mm)") {
                ParametricFootprintFields(footprint: footprint, balls: model.draft.isBallPinout
                    ? model.draft.pinDesignators : nil)
                Button("Remove this footprint", role: .destructive) { model.draft.inlineFootprint = nil }
            }
        } else {
            Section("The shape") {
                Picker("Shape", selection: $style) {
                    ForEach(ParametricFootprintDraft.Style.allCases, id: \.self) { style in
                        Text("\(style.title) — \(style.examples)").tag(style)
                    }
                }
                FootprintDiagram(style: style)
                Button("Make a \(style.title.lowercased()) footprint") { model.startParametricFootprint(style) }
                    .disabled(model.draft.pinCount == 0)
                if model.draft.pinCount == 0 {
                    Text("Give the part its pins first.").foregroundStyle(.secondary)
                }
            }
            .onAppear { if model.draft.isBallPinout { style = .grid } }
        }
    }

    // MARK: Pad by pad

    @ViewBuilder private var pads: some View {
        if model.draft.customFootprint != nil {
            Section("Pad table") {
                CustomFootprintEditor(
                    footprint: Binding(get: { model.draft.customFootprint ?? CustomFootprintDraft.starter(id: "", pins: 1) },
                                       set: { model.draft.customFootprint = $0 }),
                    pinNumbers: model.draft.pinNumbers,
                    onRemove: { model.draft.customFootprint = nil })
            }
        } else {
            Section("Pad table") {
                Text("For shapes the other two cannot make: pads of different sizes, mounting legs, slots.")
                    .foregroundStyle(.secondary)
                Button("Start a pad table") { model.startPadTable() }
                    .disabled(model.draft.pinCount == 0)
            }
        }
    }
}

/// The numbers of a parametric footprint, named as the sketch names them.
struct ParametricFootprintFields: View {
    @Binding var footprint: ParametricFootprintDraft
    /// The pinout's ball names, for a ball pinout.
    let balls: [String]?

    var body: some View {
        TextField("Description", text: $footprint.description)
        switch footprint.style {
        case .dual:
            number("Pitch", $footprint.pitch, help: "Centre to centre of two pads next to each other in a row")
            number("Row spacing", $footprint.span, help: "Centre to centre of the two rows of pads")
            number("Pad length", $footprint.padWidth, help: "Across the row, the long side of an SMD pad")
            number("Pad width", $footprint.padHeight, help: "Along the row")
            if footprint.pins % 2 == 1 || footprint.perSide != nil {
                whole("Pads per row", Binding(get: { footprint.perSide ?? (footprint.pins + 1) / 2 },
                                               set: { footprint.perSide = $0 }))
            }
        case .quad:
            LabeledContent("Pads per side") {
                HStack {
                    ForEach(Array(["left", "bottom", "right", "top"].enumerated()), id: \.offset) { index, name in
                        TextField(name, value: Binding(get: { footprint.sides[index] },
                                                       set: { footprint.sides[index] = max($0, 0) }),
                                  format: .number.grouping(.never))
                            .frame(width: 52)
                    }
                }
            }
            number("Pitch", $footprint.pitch, help: "Centre to centre of two pads next to each other")
            number("Spacing across", $footprint.spanX, help: "Centre to centre of the left and right columns")
            number("Spacing down", $footprint.spanY, help: "Centre to centre of the top and bottom rows")
            number("Pad length", $footprint.padWidth, help: "Pointing at the body")
            number("Pad width", $footprint.padHeight, help: "Along the side")
        case .grid:
            whole("Rows", $footprint.rows)
            whole("Columns", $footprint.columns)
            number("Pitch", $footprint.pitch, help: "Centre to centre of two balls next to each other")
            number("Ball pad", Binding(get: { footprint.padWidth },
                                       set: { footprint.padWidth = $0; footprint.padHeight = $0 }),
                   help: "The pad's diameter on the board")
            LabeledContent("Balls", value: "\(footprint.balls.count) of \(footprint.rows * footprint.columns) places")
            if let balls, balls != footprint.balls {
                Button("Use the pinout's \(balls.count) balls") {
                    footprint.balls = balls
                    footprint.pins = balls.count
                    let size = ParametricFootprintDraft.gridSize(balls: balls, count: balls.count)
                    footprint.rows = max(footprint.rows, size.rows)
                    footprint.columns = max(footprint.columns, size.columns)
                }
            }
        case .header:
            Picker("Rows", selection: Binding(get: { footprint.rows }, set: { rows in
                footprint.rows = rows
                footprint.rowPitch = rows == 2 ? (footprint.rowPitch ?? footprint.pitch) : nil
            })) {
                Text("1").tag(1)
                Text("2").tag(2)
            }
            number("Pitch", $footprint.pitch, help: "Centre to centre of two pins next to each other in a row")
            if footprint.rows == 2 {
                number("Row pitch", Binding(get: { footprint.rowPitch ?? footprint.pitch },
                                            set: { footprint.rowPitch = $0 }),
                       help: "Centre to centre of the two rows")
            }
            number("First row offset", $footprint.firstRowY, help: "Row 1's distance from the body's centre; 0 for a straight header")
            number("Pad", Binding(get: { footprint.padWidth },
                                  set: { footprint.padWidth = $0; footprint.padHeight = $0 }),
                   help: "The copper pad's diameter")
        }
        if footprint.style == .header || footprint.drill != nil {
            number("Drill", Binding(get: { footprint.drill ?? 0 }, set: { footprint.drill = $0 > 0 ? $0 : nil }),
                   help: "The hole's diameter; 0 for surface mount")
        } else {
            Toggle("Through-hole (drilled pads)", isOn: Binding(
                get: { footprint.drill != nil },
                set: { footprint.drill = $0 ? 0.8 : nil }))
        }
        LabeledContent("Body") {
            HStack {
                TextField("Body width", value: $footprint.bodyWidth, format: .number.grouping(.never))
                    .labelsHidden().frame(width: 70)
                Text("wide ×")
                TextField("Body height", value: $footprint.bodyHeight, format: .number.grouping(.never))
                    .labelsHidden().frame(width: 70)
                Text("high")
            }
        }
    }

    private func number(_ label: String, _ value: Binding<Double>, help: String) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            TextField(label, value: value, format: .number.grouping(.never))
            Text(help).font(.caption).foregroundStyle(.secondary)
        }
    }

    private func whole(_ label: String, _ value: Binding<Int>) -> some View {
        TextField(label, value: value, format: .number.grouping(.never))
    }
}

extension PartEditorModel {
    /// Chooses a library footprint; a custom one the part had is removed.
    func useLibraryFootprint(_ id: String) {
        var next = draft
        next.inlineFootprint = nil
        next.footprint = id
        draft = next
    }

    /// Gives the part a footprint of a parametric style to fill in, for the
    /// pinout's pins (its balls, for a ball grid).
    func startParametricFootprint(_ style: ParametricFootprintDraft.Style) {
        let id = CustomFootprintDraft.id(forPart: draft.uuid)
        draft.inlineFootprint = .parametric(ParametricFootprintDraft.starter(
            style: style, id: id, pins: draft.pinCount, balls: draft.isBallPinout ? draft.pinDesignators : []))
    }

    /// Gives the part a pad table to fill in: one pad per pin (per ball, for
    /// a ball pinout).
    func startPadTable() {
        var custom = CustomFootprintDraft.starter(id: CustomFootprintDraft.id(forPart: draft.uuid), pins: draft.pinCount)
        if draft.isBallPinout {
            for (index, ball) in draft.pinDesignators.enumerated() where index < custom.pads.count {
                custom.pads[index].pin = 0
                custom.pads[index].ball = ball
            }
        } else {
            let numbers = draft.pinNumbers
            for index in custom.pads.indices where index < numbers.count { custom.pads[index].pin = numbers[index] }
        }
        draft.inlineFootprint = .explicit(custom)
    }

    /// "Start from this one": a library footprint copied into the part, as
    /// numbers when it is one of the parametric styles and as a pad table
    /// otherwise. Returns the tab that now edits it.
    @discardableResult
    func startFromLibraryFootprint(_ footprint: Footprint) -> FootprintStepPage.Choice {
        let id = CustomFootprintDraft.id(forPart: draft.uuid)
        if let entry = BundledFootprints.entry(id: footprint.id), var parametric = ParametricFootprintDraft(json: entry) {
            parametric.id = id
            draft.inlineFootprint = .parametric(parametric)
            return .make
        }
        var explicit = CustomFootprintDraft(footprint: footprint)
        explicit.id = id
        draft.inlineFootprint = .explicit(explicit)
        return .pads
    }
}
