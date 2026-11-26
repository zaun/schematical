import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `PartList` on hand-built parts, with no document (plan-68-F): placing
/// and naming, options, sockets and footprint choices, turns, switches,
/// units and footprints on the PCB.
@MainActor
final class PartListTests: XCTestCase {
    private func list(_ names: String...) throws -> PartList {
        var value = PartList(chips: [], nextReference: 1)
        for (i, name) in names.enumerated() {
            let part = try TestSupport.part(name)
            _ = value.place(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 200 * i, y: 0),
                            color: nil, footprintID: nil, gender: nil, ohms: nil, farads: nil, grid: 1.27)
        }
        return value
    }

    func testPlacingAndNaming() throws {
        var value = try list("SN74HC00N", "SN74HC00N", "GENERIC-LED")
        XCTAssertEqual(value.chips.map(\.reference), ["U1", "U2", "D1"])
        XCTAssertEqual(value.nextReference, 3)
        XCTAssertNotNil(value.chips[0].board, "a footprint spot beside the others")
        XCTAssertEqual(value.takeReference(for: value.chips[0].part, gender: nil, alsoUsed: ["U3"]), "U4")
        let id = value.chips[0].id
        XCTAssertNotNil(value.rename(id, prefix: "1U", number: 1), "a bad prefix")
        XCTAssertEqual(value.rename(id, prefix: "U", number: 2), "U2 is already used by another part.")
        XCTAssertNotNil(value.rename(id, prefix: "U", number: nil), "a number is needed")
        XCTAssertNil(value.rename(UUID(), prefix: "U", number: 9))
        XCTAssertNil(value.rename(id, prefix: "U", number: 9))
        XCTAssertEqual(value.chips[0].reference, "U9")
        value.resetReferences()
        XCTAssertEqual(value.chips.map(\.reference), ["U1", "U2", "D1"])
        XCTAssertEqual(value.nextReference, 3)
        XCTAssertNil(value.index(UUID()))
        // Two parts at one spot are numbered by their references.
        value.chips[1].origin = value.chips[0].origin
        value.chips[0].reference = "U7"
        value.chips[1].reference = "U6"
        value.resetReferences()
        XCTAssertEqual(value.chips.prefix(2).map(\.reference), ["U2", "U1"])
    }

    func testOptions() throws {
        var value = try list("GENERIC-LED", "GENERIC-HEADER-1X04", "SN74HC00N")
        let led = value.chips[0].id, header = value.chips[1].id, gate = value.chips[2].id
        let color = try XCTUnwrap(value.chips[0].part.colors.last)
        value.setColor(led, color)
        value.setColor(led, "no such colour")
        value.setColor(gate, color)
        XCTAssertEqual(value.chips[0].color, color)
        value.setShowsReference(led, false)
        XCTAssertFalse(value.chips[0].showsReference)
        let gender = try XCTUnwrap(value.chips[1].part.genders.last)
        value.setGender(header, gender)
        value.setGender(led, gender)
        XCTAssertEqual(value.chips[1].gender, gender)
        value.setOhms(led, 1000)
        value.setFarads(led, 1e-6)
        value.setMemoryFile(led, nil)
        XCTAssertNil(value.chips[0].ohms)
        let bigger = try TestSupport.part("GENERIC-HEADER-1X05")
        XCTAssertTrue(value.canResizeHeader(header, to: bigger))
        XCTAssertFalse(value.canResizeHeader(header, to: value.chips[1].part))
        XCTAssertFalse(value.canResizeHeader(led, to: bigger))
        XCTAssertFalse(value.canResizeHeader(UUID(), to: bigger))
        let reference = value.chips[1].reference
        value.swap(header, to: bigger, symbol: nil)
        value.swap(UUID(), to: bigger, symbol: nil)
        XCTAssertEqual(value.chips[1].part.pins.count, 5)
        XCTAssertEqual(value.chips[1].reference, reference)
    }

    func testValuesSocketsAndFootprintChoices() throws {
        var value = try list("GENERIC-RESISTOR", "GENERIC-CAPACITOR", "SN74HC00N")
        let resistor = value.chips[0].id, capacitor = value.chips[1].id, gate = value.chips[2].id
        value.setOhms(resistor, 4700)
        value.setOhms(resistor, -1)
        XCTAssertEqual(value.chips[0].ohms, 4700)
        value.setFarads(capacitor, 1e-9)
        XCTAssertEqual(value.chips[1].farads, 1e-9)
        if let choice = value.chips[0].part.footprintChoices.last?.footprint.id {
            value.setFootprint(resistor, choice)
            XCTAssertEqual(value.chips[0].footprintID, choice)
        }
        value.setFootprint(resistor, "no such footprint")
        value.setFootprint(UUID(), "x")
        let socket = try TestSupport.part("GENERIC-SOCKET-DIP-14-300")
        let modes: [ChipSocket.Mode] = [.withSocket, .socketOnly]
        value.setSocket(gate, mode: modes[0], socket: socket)
        XCTAssertEqual(value.chips[2].socket?.mode, modes[0])
        value.setSocket(gate, mode: modes[0], socket: nil)
        value.setSocket(gate, mode: modes.last, socket: nil)
        XCTAssertEqual(value.chips[2].socket?.mode, modes.last)
        value.setSocket(gate, mode: nil, socket: nil)
        XCTAssertNil(value.chips[2].socket)
        value.setSocket(resistor, mode: modes[0], socket: socket)
        XCTAssertNil(value.chips[0].socket, "a resistor fits no IC socket")
        value.setSocket(UUID(), mode: nil, socket: nil)
    }

    func testTurnsAndUnits() throws {
        var value = try list("SN74HC00N", "GENERIC-LED")
        let gate = value.chips[0].id
        XCTAssertEqual(value.flip(gate), value.chips[0].symbol.canFlip)
        XCTAssertEqual(value.rotate(gate, counterclockwise: true), value.chips[0].symbol.canRotate)
        XCTAssertFalse(value.flip(UUID()))
        XCTAssertFalse(value.rotate(UUID(), counterclockwise: false))
        var turning = try list("GENERIC-RESISTOR")
        XCTAssertTrue(turning.rotate(turning.chips[0].id, counterclockwise: false))
        XCTAssertEqual(turning.chips[0].rotation, 90)
        XCTAssertTrue(value.setSplit(gate, true))
        XCTAssertFalse(value.setSplit(gate, true))
        XCTAssertFalse(value.setSplit(value.chips[1].id, true), "an LED has no units")
        XCTAssertFalse(value.flip(gate), "a part drawn as its units flips by unit")
        let key = try XCTUnwrap(value.chips[0].unitPlacements.first?.key)
        let ref = UnitRef(chipID: gate, key: key)
        XCTAssertNotNil(value.placementIndex(ref))
        XCTAssertNil(value.placementIndex(UnitRef(chipID: gate, key: "nope")))
        value.moveUnit(ref, to: CGPoint(x: 401, y: 399))
        XCTAssertEqual(value.chips[0].unitPlacements.first?.origin, SchematicGrid.snap(CGPoint(x: 401, y: 399)))
        value.flipUnit(ref)
        XCTAssertTrue(value.chips[0].unitPlacements.first?.isFlipped ?? false)
        if value.canRotateUnit(ref) { value.rotateUnit(ref, counterclockwise: false) }
        XCTAssertFalse(value.canRotateUnit(UnitRef(chipID: UUID(), key: key)))
        value.moveUnit(UnitRef(chipID: UUID(), key: key), to: .zero)
        value.flipUnit(UnitRef(chipID: UUID(), key: key))
        value.rotateUnit(UnitRef(chipID: UUID(), key: key), counterclockwise: true)
        let at = try XCTUnwrap(value.chips[0].unitPlacements.first?.origin)
        value.moveGroup(origins: [value.chips[1].id: .zero, UUID(): .zero], units: [ref: at, UnitRef(chipID: UUID(), key: key): at],
                        by: { CGPoint(x: $0.x + 20, y: $0.y) })
        XCTAssertEqual(value.chips[1].origin, CGPoint(x: 20, y: 0))
        value.moveGroup(origins: [value.chips[1].id: .zero], units: [ref: at], by: { CGPoint(x: $0.x + 20, y: $0.y) })
        XCTAssertTrue(value.setSplit(gate, false))
        let rect = value.chips[1].bodyRect
        XCTAssertEqual(value.chip(at: CGPoint(x: rect.midX, y: rect.midY))?.id, value.chips[1].id)
        XCTAssertEqual(value.piece(at: CGPoint(x: rect.midX, y: rect.midY))?.id, value.chips[1].id)
        XCTAssertNil(value.chip(at: CGPoint(x: -9000, y: -9000)))
    }

    func testSwitches() throws {
        let dip = try TestSupport.firstDip()
        var value = PartList(chips: [], nextReference: 1)
        let id = value.place(part: dip, symbol: symbol(for: dip), centeredAt: .zero, color: nil,
                             footprintID: nil, gender: nil, ohms: nil, farads: nil, grid: 1.27)
        XCTAssertNil(value.toggled(id, position: 99))
        XCTAssertNil(value.toggled(UUID(), position: 1))
        let toggled = try XCTUnwrap(value.toggled(id, position: 1))
        value.setSwitches(at: toggled.index, toggled.on)
        value.setSwitches(at: toggled.index, toggled.on)
        XCTAssertTrue(value.chips[0].switchesOn[0])
        let saved: [UUID: [Bool]] = [id: value.chips[0].switchesOn.map { _ in false }, UUID(): []]
        value.restoreSwitches(saved)
        XCTAssertFalse(value.chips[0].switchesOn[0])
        value.restoreSwitches([id: []])
        value.restoreSwitches(saved)

        let slide = try TestSupport.part("MSN23ANW03")
        let slideID = value.place(part: slide, symbol: symbol(for: slide), centeredAt: CGPoint(x: 300, y: 0),
                                  color: nil, footprintID: nil, gender: nil, ohms: nil, farads: nil, grid: 1.27)
        let index = try XCTUnwrap(value.index(slideID))
        let states = try XCTUnwrap(slide.slideStates)
        let closed = PlacedChip.contacts(mask: states.masks[0], count: slide.slideContacts)
        value.setSlide(at: index, state: 1, closed: closed)
        value.setSlide(at: index, state: 3, closed: PlacedChip.contacts(mask: states.masks[2], count: slide.slideContacts))
        XCTAssertEqual(value.chips[index].slideDirection, 1)

        let button = try XCTUnwrap(TestSupport.allParts.first { $0.isTactileSwitch && !$0.isNormallyClosed })
        let buttonID = value.place(part: button, symbol: symbol(for: button), centeredAt: CGPoint(x: 600, y: 0),
                                   color: nil, footprintID: nil, gender: nil, ohms: nil, farads: nil, grid: 1.27)
        let b = try XCTUnwrap(value.index(buttonID))
        value.chips[b].switchesOn[0] = true
        value.releaseAllButtons()
        XCTAssertFalse(value.chips[b].switchesOn[0])
    }

    func testFootprints() throws {
        var value = try list("SN74HC00N", "NHD-0216BZ-FL-YBW", "GENERIC-LED")
        let gate = value.chips[0].id, jack = value.chips[1].id
        let start = try XCTUnwrap(value.chips[0].board)
        value.moveFootprint(gate, to: CGPoint(x: start.centre.x + 10.1, y: start.centre.y), grid: 1.27)
        XCTAssertNotEqual(value.chips[0].board?.centre, start.centre)
        value.moveFootprintOrigin(gate, to: CGPoint(x: 50, y: 50), grid: 1.27)
        value.rotateFootprint(gate, counterclockwise: false, grid: 1.27)
        value.rotateFootprint(gate, counterclockwise: true, grid: 1.27)
        XCTAssertEqual(value.chips[0].board?.rotation, start.rotation)
        value.setFootprintSide(gate, .bottom, layers: 1, grid: 1.27)
        XCTAssertEqual(value.chips[0].board?.side, .top, "a 1-layer board has no bottom")
        value.setFootprintSide(gate, .bottom, layers: 2, grid: 1.27)
        value.setFootprintSide(gate, .bottom, layers: 2, grid: 1.27)
        XCTAssertEqual(value.chips[0].board?.side, .bottom)
        let origins = FootprintOrigin.allCases
        value.setFootprintOrigin(gate, origins.last!)
        value.setFootprintOrigin(UUID(), origins[0])
        XCTAssertEqual(value.chips[0].board?.origin, origins.last)
        value.setFootprintStyle(gate, .connector)
        XCTAssertEqual(value.chips[0].board?.style, start.style, "no connector footprint")
        value.setFootprintStyle(jack, .connector)
        XCTAssertEqual(value.chips[1].board?.style, .connector)
        value.setFootprintStyle(jack, .connector)
        value.setFootprintMarks([gate, UUID()], outline: .document, pin1: .document, reference: .document)
        value.setFootprintMarks([gate], outline: nil, pin1: nil, reference: nil)
        XCTAssertEqual(value.chips[0].board?.outline, .document)
        value.moveReference(gate, toCentre: CGPoint(x: 1, y: 1), grid: 0.635)
        value.moveReference(gate, toCentre: CGPoint(x: 1, y: 1), grid: 0.635)
        value.moveReference(UUID(), toCentre: .zero, grid: 1)
        value.resetReference(gate)
        value.resetReference(gate)
        XCTAssertEqual(value.chips[0].board?.referenceOffset, .zero)
        value.setLocked(footprint: gate, true)
        value.setLocked(footprint: UUID(), true)
        let locked = value.chips[0].board
        value.moveFootprint(gate, to: .zero, grid: 1.27)
        XCTAssertEqual(value.chips[0].board, locked, "locked")
        value.moveGroup(footprints: [gate: .zero, UUID(): .zero], by: CGSize(width: 2, height: 0))
        XCTAssertEqual(value.chips[0].board?.centre, CGPoint(x: 2, y: 0), "a group drag holds what it took")
        value.moveGroup(footprints: [gate: .zero], by: CGSize(width: 2, height: 0))
    }
}
