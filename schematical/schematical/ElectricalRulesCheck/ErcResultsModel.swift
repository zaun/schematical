//
//  ErcResultsModel.swift
//  schematical
//
//  What the Electrical Rules Check window shows (plan-63-F): the violations of
//  the drawing and the excluded ones, the selected lines, and what selecting a
//  line does on the schematic. A plain object the window owns, so tests drive it.
//

import CoreGraphics
import Foundation
import Observation

@MainActor
@Observable
final class ErcResultsModel {
    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime when a
    /// closing document frees this.
    nonisolated deinit {}

    let document: SchematicDocument
    /// The part library the check reads (the document's own when nil).
    let library: PartLibraryHandle?
    /// The violations shown, numbered from 1.
    private(set) var violations: [ErcViolation] = []
    /// The excluded violations, numbered from 1.
    private(set) var excluded: [ErcViolation] = []
    /// The selected lines of the list on show (`ErcViolation.id`).
    var selection: Set<Int> = []
    /// Whether the excluded list is on show, not the violations.
    var showsExcluded = false
    private(set) var hasRun = false

    init(document: SchematicDocument, library: PartLibraryHandle? = nil) {
        self.document = document
        self.library = library
    }

    /// The list on show.
    var rows: [ErcViolation] { showsExcluded ? excluded : violations }

    var errorCount: Int { violations.filter { $0.severity == .error }.count }
    var warningCount: Int { violations.filter { $0.severity == .warning }.count }

    /// Runs the check again. A selection that names lines no longer there is dropped.
    func run() {
        let all = document.electricalRulesCheck(library: library, includeExcluded: true)
        violations = Self.numbered(all.filter { !$0.isExcluded })
        excluded = Self.numbered(all.filter(\.isExcluded))
        let ids = Set(rows.map(\.id))
        if !selection.isSubset(of: ids) { selection = selection.intersection(ids) }
        hasRun = true
        publishMarkers()
    }

    /// Puts a mark on the schematic at each violation shown (never an excluded one).
    private func publishMarkers() {
        document.ercMarkers = violations.map { ErcMarker(point: $0.point, severity: $0.severity, key: $0.key) }
        document.ercSelectedKeys = Set(selectedRows.map(\.key))
    }

    /// Takes the marks off the schematic (the window closes).
    func clearMarkers() {
        document.ercMarkers = []
        document.ercSelectedKeys = []
    }

    private static func numbered(_ list: [ErcViolation]) -> [ErcViolation] {
        var list = list
        for i in list.indices { list[i].index = i + 1 }
        return list
    }

    /// The selected lines, in list order.
    var selectedRows: [ErcViolation] { rows.filter { selection.contains($0.id) } }

    /// What selecting the lines selects on the schematic, and where the first one is.
    func items(of ids: Set<Int>) -> (items: Set<SchematicItem>, point: CGPoint?) {
        let chosen = rows.filter { ids.contains($0.id) }
        return (chosen.reduce(into: Set<SchematicItem>()) { $0.formUnion($1.items) }, chosen.first?.point)
    }

    /// Selects what the chosen lines name on the schematic.
    func select(_ ids: Set<Int>) {
        let target = items(of: ids)
        if !target.items.isEmpty { document.selectSchematicItems(target.items) }
        document.ercSelectedKeys = Set(rows.filter { ids.contains($0.id) }.map(\.key))
        if let point = target.point { document.focusErc(on: point) }
    }

    /// Excludes the selected lines (one undo step); the list reruns.
    func excludeSelected() {
        guard !showsExcluded else { return }
        document.excludeErc(Set(selectedRows.map(\.key)))
        selection = []
        run()
    }

    /// Puts the selected excluded lines back.
    func includeSelected() {
        guard showsExcluded else { return }
        document.includeErc(Set(selectedRows.map(\.key)))
        selection = []
        run()
    }
}
