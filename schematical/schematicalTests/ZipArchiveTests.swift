import Foundation
import XCTest
@testable import schematical

/// The stored (uncompressed) zip writer behind the Gerber upload (plan-68-Z).
final class ZipArchiveTests: XCTestCase {
    private func le16(_ data: Data, _ at: Int) -> Int { Int(data[at]) | Int(data[at + 1]) << 8 }
    private func le32(_ data: Data, _ at: Int) -> UInt32 { (0..<4).reduce(0) { $0 | UInt32(data[at + $1]) << (8 * $1) } }

    func testTheCRCIsTheZipOne() {
        XCTAssertEqual(ZipArchive.crc32(Data("123456789".utf8)), 0xCBF4_3926)
        XCTAssertEqual(ZipArchive.crc32(Data()), 0)
    }

    func testEntriesHeadersAndDirectory() throws {
        var parts = DateComponents()
        (parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second) = (2026, 11, 26, 13, 45, 30)
        let date = try XCTUnwrap(Calendar.current.date(from: parts))
        let one = Data("G04 top*".utf8), two = Data("M48".utf8)
        let zip = ZipArchive.stored([(name: "top.gtl", data: one), (name: "drill.drl", data: two)], date: date)

        // First local header, then its name and the bytes, stored.
        XCTAssertEqual(le32(zip, 0), 0x0403_4B50)
        XCTAssertEqual(le16(zip, 6), 0x0800)
        XCTAssertEqual(le16(zip, 8), 0)
        XCTAssertEqual(le16(zip, 10), (13 << 11) | (45 << 5) | 15)
        XCTAssertEqual(le16(zip, 12), ((2026 - 1980) << 9) | (11 << 5) | 26)
        XCTAssertEqual(le32(zip, 14), ZipArchive.crc32(one))
        XCTAssertEqual(le32(zip, 18), UInt32(one.count))
        XCTAssertEqual(le16(zip, 26), 7)
        XCTAssertEqual(String(decoding: zip[30..<37], as: UTF8.self), "top.gtl")
        XCTAssertEqual(zip[37..<(37 + one.count)], one)
        let second = 37 + one.count
        XCTAssertEqual(le32(zip, second), 0x0403_4B50)

        // The end record: two entries, and where the directory is.
        let end = zip.count - 22
        XCTAssertEqual(le32(zip, end), 0x0605_4B50)
        XCTAssertEqual(le16(zip, end + 8), 2)
        XCTAssertEqual(le16(zip, end + 10), 2)
        let directory = Int(le32(zip, end + 16)), size = Int(le32(zip, end + 12))
        XCTAssertEqual(directory + size, end)
        XCTAssertEqual(le32(zip, directory), 0x0201_4B50)
        XCTAssertEqual(le32(zip, directory + 42), 0)
        let secondEntry = directory + 46 + 7
        XCTAssertEqual(le32(zip, secondEntry), 0x0201_4B50)
        XCTAssertEqual(Int(le32(zip, secondEntry + 42)), second)
        XCTAssertEqual(String(decoding: zip[(secondEntry + 46)..<(secondEntry + 55)], as: UTF8.self), "drill.drl")
    }

    /// Before 1980 the date is 1980's: a DOS date cannot go earlier.
    func testAnEarlyDateIs1980() {
        let zip = ZipArchive.stored([(name: "a", data: Data())], date: Date(timeIntervalSince1970: 0))
        XCTAssertEqual(le16(zip, 12) >> 9, 0)
        XCTAssertEqual(zip.count, 30 + 1 + 46 + 1 + 22)
    }
}
