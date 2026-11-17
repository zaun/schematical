import SwiftUI
import XCTest
@testable import schematical

@MainActor
final class BoardDetailsSheetTests: XCTestCase {
    func testCardTabOnlyForACardWithSettings() {
        let doc = TestSupport.newDocument()
        XCTAssertEqual(BoardDetailsSheet.tabs(for: doc.board), [.stackup, .planes, .nets, .classes, .groups])
        doc.setBoardType(.customCard)
        XCTAssertEqual(BoardDetailsSheet.tabs(for: doc.board).first, .card)
        if let withOptions = BoardType.allCases.first(where: \.hasOptions) {
            doc.setBoardType(withOptions)
            XCTAssertEqual(BoardDetailsSheet.tabs(for: doc.board).first, .card)
        }
    }

    func testSummaryNamesTypeSizeAndLayers() {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 100, height: 80)
        doc.setLayerCount(4)
        XCTAssertEqual(BoardDetailsSheet.summary(of: doc.board), "Custom · 100 × 80 mm · 4 layers")
    }

    func testNetStatusCountsConnections() {
        XCTAssertEqual(NetRowView.status(pads: 6, unrouted: 2), "2 of 5 unrouted")
        XCTAssertEqual(NetRowView.status(pads: 2, unrouted: 0), "0 of 1 unrouted")
        XCTAssertEqual(NetRowView.status(pads: 1, unrouted: 0), "—")
    }

    func testSheetAndSidebarBuild() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let sheet = BoardDetailsSheet(document: doc, highlightedNet: .constant(nil), onClose: {})
        XCTAssertNotNil(ImageRenderer(content: sheet).cgImage)
        let sidebar = PCBSidebarView(document: doc).frame(width: 350, height: 700)
        XCTAssertNotNil(ImageRenderer(content: sidebar).cgImage)
    }
}
