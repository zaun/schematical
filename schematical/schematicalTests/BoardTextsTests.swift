import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `BoardTexts` on hand-built texts, with no document (plan-68-E).
@MainActor
final class BoardTextsTests: XCTestCase {
    func testEdits() {
        var value = BoardTexts(texts: [])
        let text = value.add(at: CGPoint(x: 1.3, y: 2.6), grid: 1)
        XCTAssertEqual(text.anchor, CGPoint(x: 1, y: 3))
        value.setText(text.id, "")
        value.setText(text.id, "J1")
        value.setText(UUID(), "X")
        XCTAssertEqual(value.texts[0].text, "J1")
        value.setSize(text.id, 100_000)
        value.setSize(UUID(), 1)
        XCTAssertEqual(value.texts[0].size, BoardText.sizeRange.upperBound)
        value.setLayer(text.id, .document)
        value.setLayer(UUID(), .document)
        XCTAssertEqual(value.texts[0].layer, .document)
        value.setSide(text.id, .bottom, layers: 1)
        XCTAssertEqual(value.texts[0].side, .top, "a 1-layer board has no bottom")
        value.setSide(text.id, .bottom, layers: 2)
        value.setSide(text.id, .bottom, layers: 2)
        XCTAssertEqual(value.texts[0].side, .bottom)
        value.move(text.id, to: CGPoint(x: 10.2, y: 10.7), grid: 1)
        XCTAssertEqual(value.texts[0].anchor, CGPoint(x: 10, y: 11))
        value.setRotation(text.id, 45)
        value.setRotation(text.id, 180)
        XCTAssertEqual(value.texts[0].rotation, 180)
        value.rotate(text.id, counterclockwise: true)
        XCTAssertEqual(value.texts[0].rotation, 90)
        value.rotate(text.id, counterclockwise: false)
        XCTAssertEqual(value.texts[0].rotation, 180)
        value.setLocked(text.id, true)
        value.setLocked(UUID(), true)
        value.move(text.id, to: .zero, grid: 1)
        value.setRotation(text.id, 0)
        value.rotate(text.id, counterclockwise: false)
        XCTAssertEqual(value.texts[0].anchor, CGPoint(x: 10, y: 11), "locked")
        XCTAssertEqual(value.texts[0].rotation, 180)
        value.move([text.id: CGPoint(x: 1, y: 1), UUID(): .zero], by: { CGPoint(x: $0.x + 1, y: $0.y) })
        XCTAssertEqual(value.texts[0].anchor, CGPoint(x: 2, y: 1), "a group drag moves even a locked text it holds")
        value.move([text.id: CGPoint(x: 1, y: 1)], by: { CGPoint(x: $0.x + 1, y: $0.y) })
        let pasted = value.paste(value.texts, offset: CGSize(width: 5, height: 0), grid: 1, layers: 1)
        XCTAssertEqual(pasted.count, 1)
        XCTAssertEqual(pasted[0].side, .top)
        XCTAssertFalse(pasted[0].locked)
        XCTAssertEqual(value.paste(value.texts.prefix(1).map { $0 }, offset: .zero, grid: 1, layers: 2)[0].side, .bottom)
        value.remove([text.id])
        XCTAssertEqual(value.texts.count, 2)
    }

    func testHits() {
        var value = BoardTexts(texts: [])
        let top = value.add(at: CGPoint(x: 10, y: 10), grid: 1)
        let bottom = value.add(at: CGPoint(x: 10, y: 10), grid: 1)
        value.setSide(bottom.id, .bottom, layers: 2)
        let rect = value.texts[0].boardRect
        let inside = CGPoint(x: rect.midX, y: rect.midY)
        XCTAssertEqual(value.text(at: inside, visible: BoardVisibility())?.id, top.id, "Top before Bottom")
        value.remove([top.id])
        XCTAssertEqual(value.text(at: inside, visible: BoardVisibility())?.id, bottom.id)
        XCTAssertNil(value.text(at: CGPoint(x: -100, y: -100), visible: BoardVisibility()))
    }
}
