import Foundation

/// A minimal zip writer: files stored uncompressed (method 0), names in
/// UTF-8, no folders. Enough for a fab's Gerber upload.
nonisolated enum ZipArchive {
    /// A zip of `entries` (name, bytes), in order, dated `date`.
    static func stored(_ entries: [(name: String, data: Data)], date: Date = Date()) -> Data {
        let (time, day) = dosDateTime(date)
        var archive = Data()
        var directory = Data()
        for (name, data) in entries {
            let nameBytes = Data(name.utf8)
            let crc = crc32(data)
            let offset = UInt32(archive.count)
            // Local file header.
            archive.append(le32: 0x0403_4B50)
            archive.append(le16: 20)                 // version needed
            archive.append(le16: 0x0800)             // UTF-8 names
            archive.append(le16: 0)                  // stored
            archive.append(le16: time)
            archive.append(le16: day)
            archive.append(le32: crc)
            archive.append(le32: UInt32(data.count))
            archive.append(le32: UInt32(data.count))
            archive.append(le16: UInt16(nameBytes.count))
            archive.append(le16: 0)                  // extra
            archive.append(nameBytes)
            archive.append(data)
            // Its central directory entry.
            directory.append(le32: 0x0201_4B50)
            directory.append(le16: 20)               // version made by
            directory.append(le16: 20)
            directory.append(le16: 0x0800)
            directory.append(le16: 0)
            directory.append(le16: time)
            directory.append(le16: day)
            directory.append(le32: crc)
            directory.append(le32: UInt32(data.count))
            directory.append(le32: UInt32(data.count))
            directory.append(le16: UInt16(nameBytes.count))
            directory.append(le16: 0)                // extra
            directory.append(le16: 0)                // comment
            directory.append(le16: 0)                // disk
            directory.append(le16: 0)                // internal attributes
            directory.append(le32: 0)                // external attributes
            directory.append(le32: offset)
            directory.append(nameBytes)
        }
        let directoryOffset = UInt32(archive.count)
        archive.append(directory)
        // End of central directory.
        archive.append(le32: 0x0605_4B50)
        archive.append(le16: 0)
        archive.append(le16: 0)
        archive.append(le16: UInt16(entries.count))
        archive.append(le16: UInt16(entries.count))
        archive.append(le32: UInt32(directory.count))
        archive.append(le32: directoryOffset)
        archive.append(le16: 0)
        return archive
    }

    /// The zip CRC-32 (IEEE, reflected).
    static func crc32(_ data: Data) -> UInt32 {
        var crc: UInt32 = 0xFFFF_FFFF
        for byte in data {
            crc = table[Int((crc ^ UInt32(byte)) & 0xFF)] ^ (crc >> 8)
        }
        return crc ^ 0xFFFF_FFFF
    }

    private static let table: [UInt32] = (0..<256).map { n in
        var c = UInt32(n)
        for _ in 0..<8 { c = c & 1 != 0 ? 0xEDB8_8320 ^ (c >> 1) : c >> 1 }
        return c
    }

    /// MS-DOS time and date, local time; 1980 at the earliest.
    private static func dosDateTime(_ date: Date) -> (time: UInt16, date: UInt16) {
        let c = Calendar.current.dateComponents([.year, .month, .day, .hour, .minute, .second], from: date)
        let year: Int = max((c.year ?? 1980) - 1980, 0)
        let hour: Int = c.hour ?? 0, minute: Int = c.minute ?? 0, second: Int = c.second ?? 0
        let month: Int = c.month ?? 1, dayOfMonth: Int = c.day ?? 1
        let time = UInt16((hour << 11) | (minute << 5) | (second / 2))
        let day = UInt16((year << 9) | (month << 5) | dayOfMonth)
        return (time, day)
    }
}

private extension Data {
    nonisolated mutating func append(le16 value: UInt16) {
        append(contentsOf: [UInt8(value & 0xFF), UInt8(value >> 8)])
    }

    nonisolated mutating func append(le32 value: UInt32) {
        append(contentsOf: (0..<4).map { UInt8((value >> (8 * $0)) & 0xFF) })
    }
}
