import AppKit
import Combine
import SwiftUI
import XCTest
@testable import schematical

/// The Part Editor's field controls (`FieldEditors.swift`, plan-68-AD): the
/// bindings into JSON values, and each control rendered and edited.
@MainActor
final class FieldEditorsTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test's window must not quit the host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    // MARK: Bindings

    /// A value the bindings read and write.
    private final class Box<Value> {
        var value: Value
        init(_ value: Value) { self.value = value }
        nonisolated deinit {}
        var binding: Binding<Value> { Binding(get: { self.value }, set: { self.value = $0 }) }
    }

    func testKeyAndObjectBindings() {
        let object = Box<[String: JSONValue]>(["a": .number(1)])
        let key = object.binding.key("a")
        XCTAssertEqual(key.wrappedValue, .number(1))
        key.wrappedValue = nil
        XCTAssertEqual(object.value, [:])

        let value = Box<JSONValue?>(nil)
        let optional = value.binding.object(removeWhenEmpty: true)
        XCTAssertEqual(optional.wrappedValue, [:])
        optional.wrappedValue = ["b": .bool(true)]
        XCTAssertEqual(value.value, .object(["b": .bool(true)]))
        XCTAssertEqual(optional.wrappedValue, ["b": .bool(true)])
        optional.wrappedValue = [:]
        XCTAssertNil(value.value)
        value.binding.object(removeWhenEmpty: false).wrappedValue = [:]
        XCTAssertEqual(value.value, .object([:]))
    }

    func testArrayStringNumberBindings() {
        let value = Box<JSONValue?>(.string("x"))
        XCTAssertEqual(value.binding.array().wrappedValue, [])
        value.binding.array().wrappedValue = [.number(2)]
        XCTAssertEqual(value.binding.array().wrappedValue, [.number(2)])

        XCTAssertEqual(value.binding.string(removeWhenEmpty: true).wrappedValue, "")
        value.value = .string("hi")
        XCTAssertEqual(value.binding.string(removeWhenEmpty: true).wrappedValue, "hi")
        value.binding.string(removeWhenEmpty: false).wrappedValue = ""
        XCTAssertEqual(value.value, .string(""))
        value.binding.string(removeWhenEmpty: true).wrappedValue = ""
        XCTAssertNil(value.value)

        XCTAssertNil(value.binding.number().wrappedValue)
        value.binding.number().wrappedValue = 3.5
        XCTAssertEqual(value.value, .number(3.5))
        XCTAssertEqual(value.binding.number().wrappedValue, 3.5)
        value.binding.number().wrappedValue = nil
        XCTAssertNil(value.value)
    }

    func testBoolTagVoltBoundAndShapeIndex() {
        let value = Box<JSONValue?>(nil)
        XCTAssertNil(value.binding.boolTag().wrappedValue)
        value.binding.boolTag().wrappedValue = 1
        XCTAssertEqual(value.value, .bool(true))
        XCTAssertEqual(value.binding.boolTag().wrappedValue, 1)
        value.binding.boolTag().wrappedValue = 0
        XCTAssertEqual(value.binding.boolTag().wrappedValue, 0)
        value.binding.boolTag().wrappedValue = nil
        XCTAssertNil(value.value)

        XCTAssertNil(value.binding.voltBound(0).wrappedValue)
        value.binding.voltBound(1).wrappedValue = 5
        XCTAssertEqual(value.value, .array([.number(0), .number(5)]))
        value.binding.voltBound(0).wrappedValue = nil
        XCTAssertEqual(value.value, .array([.number(0), .number(5)]))
        value.binding.voltBound(0).wrappedValue = 1.8
        XCTAssertEqual(value.binding.voltBound(0).wrappedValue, 1.8)
        XCTAssertEqual(value.binding.voltBound(1).wrappedValue, 5)
        value.value = .array([.string("a"), .number(1)])
        XCTAssertNil(value.binding.voltBound(0).wrappedValue)

        let types: [FieldType] = [.integer, .pin]
        value.value = nil
        XCTAssertEqual(value.binding.shapeIndex(of: types).wrappedValue, 0)
        value.value = .string("A0")
        XCTAssertEqual(value.binding.shapeIndex(of: types).wrappedValue, 1)
        value.value = .bool(true)
        XCTAssertEqual(value.binding.shapeIndex(of: types).wrappedValue, 0)
        value.binding.shapeIndex(of: types).wrappedValue = 1
        XCTAssertEqual(value.value, .string(""))
    }

    func testArrayElementBindings() {
        let items = Box<[JSONValue]>([.string("a"), .number(2), .object(["k": .null])])
        XCTAssertEqual(items.binding.element(0).wrappedValue, .string("a"))
        XCTAssertNil(items.binding.element(9).wrappedValue)
        items.binding.element(0).wrappedValue = nil
        XCTAssertEqual(items.value[0], .null)
        items.binding.element(9).wrappedValue = .bool(true)
        XCTAssertEqual(items.value.count, 3)

        XCTAssertEqual(items.binding.string(0).wrappedValue, "")
        items.binding.string(0).wrappedValue = "b"
        XCTAssertEqual(items.binding.string(0).wrappedValue, "b")
        items.binding.string(9).wrappedValue = "c"
        XCTAssertEqual(items.value.count, 3)

        XCTAssertEqual(items.binding.number(1).wrappedValue, 2)
        XCTAssertNil(items.binding.number(0).wrappedValue)
        items.binding.number(1).wrappedValue = nil
        XCTAssertEqual(items.value[1], .number(0))
        items.binding.number(9).wrappedValue = 1

        XCTAssertEqual(items.binding.object(2).wrappedValue, ["k": .null])
        XCTAssertEqual(items.binding.object(0).wrappedValue, [:])
        items.binding.object(2).wrappedValue = ["j": .number(1)]
        XCTAssertEqual(items.value[2], .object(["j": .number(1)]))
        items.binding.object(9).wrappedValue = [:]
        XCTAssertEqual(items.value.count, 3)
    }

    func testFieldEditingHelpers() {
        XCTAssertEqual(FieldEditing.options(["a", "b"], current: .string("c")), ["a", "b", "c"])
        XCTAssertEqual(FieldEditing.options(["a", "b"], current: .string("a")), ["a", "b"])
        XCTAssertEqual(FieldEditing.options(["a"], current: .string("")), ["a"])
        XCTAssertEqual(FieldEditing.options(["a"], current: nil), ["a"])

        XCTAssertEqual(FieldEditing.adding(" HC ", entry: .number(0), to: [:]), ["HC": .number(0)])
        XCTAssertNil(FieldEditing.adding("  ", entry: .number(0), to: [:]))
        XCTAssertNil(FieldEditing.adding("HC", entry: .number(0), to: ["HC": .null]))

        XCTAssertEqual(FieldEditing.parse("  ", required: false), .success(nil))
        XCTAssertEqual(FieldEditing.parse("  ", required: true), .failure(FieldEditing.JSONTextError()))
        XCTAssertEqual(FieldEditing.parse("[1, 2]", required: true), .success(.array([.number(1), .number(2)])))
        XCTAssertEqual(FieldEditing.parse("{", required: false), .failure(FieldEditing.JSONTextError()))
        XCTAssertEqual(FieldEditing.JSONTextError().message, "Not valid JSON")
    }

    // MARK: Rendered controls

    /// The value a rendered control edits.
    final class Model: ObservableObject {
        @Published var value: JSONValue?
        init(_ value: JSONValue?) { self.value = value }
        nonisolated deinit {}
    }

    private struct ValueHost: View {
        @ObservedObject var model: Model
        let type: FieldType
        var required = false
        var example: String?

        var body: some View {
            Form {
                FieldValueEditor(label: "Thing", type: type, required: required, value: $model.value,
                                 pinNames: ["A", "B"])
                    .environment(\.fieldExample, example)
            }
        }
    }

    private func show(_ type: FieldType, _ value: JSONValue? = nil, required: Bool = false,
                      example: String? = nil) -> (ViewProbe, Model) {
        let model = Model(value)
        let probe = ViewProbe(ValueHost(model: model, type: type, required: required, example: example),
                              size: NSSize(width: 520, height: 520))
        probes.append(probe)
        return (probe, model)
    }

    func testTextAndNoteFieldsWrite() throws {
        let (text, textModel) = show(.text, example: "74HC00")
        let field = try XCTUnwrap(text.textFields().first)
        XCTAssertEqual(field.placeholderString, "74HC00")
        text.enter("SN7400", into: field)
        XCTAssertEqual(textModel.value, .string("SN7400"))

        let (note, noteModel) = show(.note, .string("old"), required: true)
        let noteField = try XCTUnwrap(note.textFields().first)
        XCTAssertEqual(noteField.stringValue, "old")
        note.enter("", into: noteField)
        XCTAssertEqual(noteModel.value, .string(""))
    }

    func testNumberAndIntegerFieldsWrite() throws {
        let (number, numberModel) = show(.number(unit: "ns"), example: "10")
        XCTAssertTrue(number.has("Thing (ns)"), "\(number.labels())")
        number.enter("12.5", into: try XCTUnwrap(number.textFields().first))
        XCTAssertEqual(numberModel.value, .number(12.5))

        let (plain, _) = show(.number(unit: ""))
        XCTAssertFalse(plain.has("Thing ("))

        let (integer, integerModel) = show(.integer, .number(3))
        let field = try XCTUnwrap(integer.textFields().first)
        XCTAssertEqual(field.stringValue, "3")
        integer.enter("8", into: field)
        XCTAssertEqual(integerModel.value, .number(8))
    }

    func testBoolPickerChooses() throws {
        let (optional, model) = show(.bool)
        let popUp = try XCTUnwrap(optional.popUps().first)
        XCTAssertEqual(popUp.itemTitles, ["—", "Yes", "No"])
        optional.choose("Yes", in: popUp)
        XCTAssertEqual(model.value, .bool(true))

        let (required, _) = show(.bool, .bool(false), required: true)
        XCTAssertEqual(try XCTUnwrap(required.popUps().first).itemTitles, ["Yes", "No"])
    }

    func testChoicePickerKeepsAFileValue() throws {
        let (probe, model) = show(.choice(["rise", "fall"]), .string("both"))
        let popUp = try XCTUnwrap(probe.popUps().first)
        XCTAssertEqual(popUp.itemTitles, ["—", "rise", "fall", "both"])
        probe.choose("fall", in: popUp)
        XCTAssertEqual(model.value, .string("fall"))

        let (required, _) = show(.choice(["rise"]), .string("rise"), required: true)
        XCTAssertEqual(try XCTUnwrap(required.popUps().first).itemTitles, ["rise"])
    }

    func testSuggestedMenuFillsTheField() throws {
        let (probe, model) = show(.suggested(["fall", "rise"]), example: "fall")
        // The suggestions' pull-down is there (its items are built only as it
        // opens on screen); the field takes any text.
        XCTAssertEqual(probe.popUps().count, 1)
        let field = try XCTUnwrap(probe.textFields().first)
        XCTAssertEqual(field.placeholderString, "fall")
        probe.enter("both", into: field)
        XCTAssertEqual(model.value, .string("both"))
    }

    func testPinPickerListsPinsAndAStrayValue() throws {
        let (probe, model) = show(.pin, .string("Z"))
        let popUp = try XCTUnwrap(probe.popUps().first)
        XCTAssertEqual(popUp.itemTitles, ["—", "A", "B", "Z"])
        probe.choose("B", in: popUp)
        XCTAssertEqual(model.value, .string("B"))

        let (required, _) = show(.pin, .string("A"), required: true)
        XCTAssertEqual(try XCTUnwrap(required.popUps().first).itemTitles, ["A", "B"])
    }

    func testPinListAddsChoosesAndRemoves() throws {
        let (probe, model) = show(.pins, .array([.string("A")]))
        try probe.press("Add")
        XCTAssertEqual(model.value, .array([.string("A"), .string("A")]))
        let second = try XCTUnwrap(probe.popUps().last)
        probe.choose("B", in: second)
        XCTAssertEqual(model.value, .array([.string("A"), .string("B")]))
        let remove = probe.buttons(labelled: "remove")
        XCTAssertEqual(remove.count, 2, "\(probe.labels())")
        probe.pressElement(remove[0])
        XCTAssertEqual(model.value, .array([.string("B")]))
    }

    func testNameListAddsAndTypes() throws {
        let (probe, model) = show(.names)
        try probe.press("Add")
        XCTAssertEqual(model.value, .array([.string("")]))
        probe.enter("ripple", into: try XCTUnwrap(probe.textFields().first))
        XCTAssertEqual(model.value, .array([.string("ripple")]))
    }

    func testNumberListAddsTypesAndRemoves() throws {
        let (probe, model) = show(.numbers(unit: "ns"), .array([.number(5)]))
        XCTAssertTrue(probe.has("Thing (ns)"))
        try probe.press("Add")
        XCTAssertEqual(model.value, .array([.number(5), .number(0)]))
        probe.enter("7", into: try XCTUnwrap(probe.textFields().last))
        XCTAssertEqual(model.value, .array([.number(5), .number(7)]))
        let remove = probe.buttons(labelled: "remove")
        probe.pressElement(try XCTUnwrap(remove.first))
        XCTAssertEqual(model.value, .array([.number(7)]))

        let (plain, _) = show(.numbers(unit: ""))
        XCTAssertTrue(plain.has("Thing"))
        XCTAssertFalse(plain.has("Thing ("))
    }

    func testVoltRangeTypesAndClears() throws {
        let (probe, model) = show(.voltRange)
        XCTAssertTrue(probe.has("Thing (V)"))
        // The clear button (xmark.circle, read "Close") shows once there is a range.
        XCTAssertEqual(probe.buttons(labelled: "close").count, 0, "\(probe.labels())")
        probe.enter("3.3", into: try XCTUnwrap(probe.textFields().last))
        XCTAssertEqual(model.value, .array([.number(0), .number(3.3)]))
        probe.pressElement(try XCTUnwrap(probe.buttons(labelled: "close").first))
        XCTAssertNil(model.value)
    }

    func testOptionalObjectAddsAndRemoves() throws {
        let fields = [Field(key: "pin", label: "Pin", type: .pin, required: true),
                      Field(key: "note", label: "Note", type: .text)]
        let (probe, model) = show(.object(fields))
        try probe.press("Add")
        XCTAssertEqual(model.value, .object(["pin": .string("")]))
        probe.choose("A", in: try XCTUnwrap(probe.popUps().first))
        XCTAssertEqual(model.value, .object(["pin": .string("A")]))
        probe.pressElement(try XCTUnwrap(probe.buttons(labelled: "remove").first))
        XCTAssertNil(model.value)

        let (required, _) = show(.object(fields), required: true)
        XCTAssertTrue(required.has("Thing"))
        XCTAssertEqual(required.buttons(labelled: "remove").count, 0)
        XCTAssertFalse(required.has("Add", role: "AXButton"))
    }

    func testListOfObjectsAddsEditsAndRemoves() throws {
        let fields = [Field(key: "name", label: "Name", type: .text, required: true)]
        let (probe, model) = show(.list(fields), .array([]))
        try probe.press("Add")
        XCTAssertEqual(model.value, .array([.object(["name": .string("")])]))
        XCTAssertTrue(probe.has("Thing 1"), "\(probe.labels())")
        probe.enter("Q0", into: try XCTUnwrap(probe.textFields().first))
        XCTAssertEqual(model.value, .array([.object(["name": .string("Q0")])]))
        probe.pressElement(try XCTUnwrap(probe.buttons(labelled: "remove").first))
        XCTAssertEqual(model.value, .array([]))
    }

    func testValueListAddsEditsAndRemoves() throws {
        let (probe, model) = show(.listOf(.text), .array([.string("a")]))
        try probe.press("Add")
        XCTAssertEqual(model.value, .array([.string("a"), .string("")]))
        probe.enter("b", into: try XCTUnwrap(probe.textFields().last))
        XCTAssertEqual(model.value, .array([.string("a"), .string("b")]))
        probe.pressElement(try XCTUnwrap(probe.buttons(labelled: "remove").first))
        XCTAssertEqual(model.value, .array([.string("b")]))
    }

    func testMapAddsANamedEntryAndRemovesIt() throws {
        let fields = [Field(key: "tpd", label: "Tpd", type: .number(unit: "ns"), required: true)]
        let (probe, model) = show(.map(keyLabel: "Family", fields))
        let add = try probe.element("Add Family", role: "AXButton")
        XCTAssertFalse(ViewProbe.isEnabled(add))
        // The new key's field is the last one (the entries come first).
        probe.enter(" HC ", into: try XCTUnwrap(probe.textFields().last))
        try probe.press("Add Family")
        XCTAssertEqual(model.value, .object(["HC": .object(["tpd": .number(0)])]))
        XCTAssertTrue(probe.has("Family: HC"), "\(probe.labels())")
        XCTAssertEqual(probe.textFields().last?.stringValue, "")
        // The same key again is refused, and stays typed.
        probe.enter("HC", into: try XCTUnwrap(probe.textFields().last))
        try probe.press("Add Family")
        XCTAssertEqual(probe.textFields().last?.stringValue, "HC")
        XCTAssertEqual(model.value, .object(["HC": .object(["tpd": .number(0)])]))
        probe.pressElement(try XCTUnwrap(probe.buttons(labelled: "remove").first))
        XCTAssertNil(model.value)
    }

    func testValueMapEntries() throws {
        let (probe, model) = show(.valueMap(keyLabel: "Pin", .integer), .object(["A": .number(1)]), required: true)
        XCTAssertTrue(probe.has("Pin: A"), "\(probe.labels())")
        probe.enter("4", into: try XCTUnwrap(probe.textFields().first))
        XCTAssertEqual(model.value, .object(["A": .number(4)]))
    }

    func testOneOfSwitchesShape() throws {
        let (probe, model) = show(.oneOf([.integer, .pin]), .number(2))
        let segments = try XCTUnwrap(probe.views(NSSegmentedControl.self).first)
        XCTAssertEqual(segments.selectedSegment, 0)
        XCTAssertEqual(probe.textFields().first?.stringValue, "2")
        probe.choose(segment: 1, in: segments)
        XCTAssertEqual(model.value, .string(""))
        XCTAssertEqual(try XCTUnwrap(probe.popUps().first).itemTitles, ["—", "A", "B"])
    }

    func testJSONTextAppliesWhenItParses() throws {
        let (probe, model) = show(.json, .array([.number(1)]))
        let editor = try XCTUnwrap(probe.views(NSTextView.self).first { $0.isEditable })
        XCTAssertEqual(try JSONValue(text: editor.string), .array([.number(1)]))
        probe.enter("{", into: editor)
        XCTAssertTrue(probe.has("Not valid JSON"), "\(probe.labels())")
        XCTAssertEqual(model.value, .array([.number(1)]))
        probe.enter("[2]", into: editor)
        XCTAssertEqual(model.value, .array([.number(2)]))
        XCTAssertFalse(probe.has("Not valid JSON"))
        probe.enter(" ", into: editor)
        XCTAssertNil(model.value)
    }

    // MARK: Fields, guided and not

    private struct FieldsHost: View {
        @ObservedObject var model: Model
        let fields: [Field]
        let guided: GuidedFields?

        var body: some View {
            Form {
                FieldsEditor(fields: fields, object: $model.value.object(removeWhenEmpty: false), pinNames: ["A"])
                    .environment(\.guidedFields, guided)
            }
        }
    }

    func testAllFieldsShowsHelpAndNoKeys() throws {
        let fields = [Field(key: "tpd", label: "Delay", type: .text, help: "Propagation delay"),
                      Field(key: "note", label: "Note", type: .text)]
        let model = Model(.object([:]))
        let probe = ViewProbe(FieldsHost(model: model, fields: fields, guided: nil))
        probes.append(probe)
        XCTAssertTrue(probe.has("Propagation delay"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Key tpd"))
        probe.enter("9", into: try XCTUnwrap(probe.textFields().first))
        XCTAssertEqual(model.value, .object(["tpd": .string("9")]))
    }

    func testGuidedFieldsUseTheGuideAndHideOtherStyles() throws {
        let schema = try XCTUnwrap(PartKinds.schemas["converter"])
        let bus = try XCTUnwrap(schema.device.first { $0.key == "bus" })
        let model = Model(.object(["bus": .object(["style": .string("spi")])]))
        let probe = ViewProbe(FieldsHost(model: model, fields: [bus],
                                         guided: GuidedFields(kind: "converter", path: "device")),
                              size: NSSize(width: 640, height: 900))
        probes.append(probe)
        // The key line under each guided field.
        XCTAssertTrue(probe.has("Key bus"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Key style"))
        // spi's keys show; i2c's do not; keys with no condition are under More.
        XCTAssertTrue(probe.has("Key sclk"))
        XCTAssertFalse(probe.has("Key sda"))
        XCTAssertTrue(probe.has("More bus settings"), "\(probe.labels())")
    }

    func testGuidedFieldWithoutAGuideShowsItsHelp() throws {
        let fields = [Field(key: "zzNoGuide", label: "Odd", type: .bool, help: "Only here")]
        XCTAssertNil(Guidance.guide(kind: "gate", path: "device.zzNoGuide"))
        let probe = ViewProbe(FieldsHost(model: Model(.object([:])), fields: fields,
                                         guided: GuidedFields(kind: "gate", path: "device")))
        probes.append(probe)
        XCTAssertTrue(probe.has("Only here"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Key zzNoGuide"))
    }

    func testAGuidedFieldShowsItsGuide() throws {
        // The first text field of any kind that has a guide.
        var found: (kind: String, field: Field, guide: FieldGuide)?
        for kind in PartKinds.schemas.keys.sorted() where found == nil {
            for field in PartKinds.schemas[kind]!.device where field.type.takesPlaceholder {
                if let guide = Guidance.guide(kind: kind, path: "device.\(field.key)") {
                    found = (kind, field, guide)
                    break
                }
            }
        }
        let (kind, field, guide) = try XCTUnwrap(found, "no guided text field")
        let probe = ViewProbe(FieldsHost(model: Model(.object([:])), fields: [field],
                                         guided: GuidedFields(kind: kind, path: "device")))
        probes.append(probe)
        XCTAssertTrue(probe.has(guide.help), "\(probe.labels())")
        XCTAssertTrue(probe.has(guide.label ?? field.label))
        if let example = guide.example {
            XCTAssertEqual(probe.textFields().first?.placeholderString, example)
        }
    }
}
