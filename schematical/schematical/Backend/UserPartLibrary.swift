import Foundation

/// A `.schpart` file in the User Library folder (plan-53-B).
nonisolated struct UserPartLibraryFile: Equatable, Sendable {
    let url: URL
    let uuid: String
    let text: String
    /// The file's creation date: when the part was added.
    let added: Date
}

/// A file in the folder that could not be read as a library part.
nonisolated struct UserPartLibraryProblem: Equatable, Identifiable, Sendable {
    let url: URL
    let reason: String

    var id: URL { url }
}

/// The folder of the User Library (plan-53-B): one `<uuid>.schpart` file per
/// part, holding the part as it was registered. It lists, writes and deletes
/// files and knows nothing else.
nonisolated final class UserPartLibrary: Sendable {
    let directory: URL

    init(directory: URL) {
        self.directory = directory
    }

    /// The app's folder: `UserLibrary` in Application Support (inside the
    /// app's container, the app being sandboxed). Under XCTest it is a fresh
    /// folder in the temporary directory, so a test run never reads or
    /// writes the owner's library.
    static var defaultDirectory: URL {
        if ProcessInfo.processInfo.environment["XCTestConfigurationFilePath"] != nil {
            return testDirectory
        }
        let support = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first
            ?? URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent("Library/Application Support")
        return support.appendingPathComponent("UserLibrary", isDirectory: true)
    }

    /// One folder per test process, emptied the first time it is asked for.
    private static let testDirectory: URL = {
        let url = URL(fileURLWithPath: NSTemporaryDirectory(), isDirectory: true)
            .appendingPathComponent("schematical-test-user-library-\(ProcessInfo.processInfo.processIdentifier)",
                                    isDirectory: true)
        try? FileManager.default.removeItem(at: url)
        return url
    }()

    /// Every `.schpart` file in the folder, in file-name order; the ones that
    /// cannot be read are problems. A missing or unreadable folder gives
    /// nothing.
    func files() -> (files: [UserPartLibraryFile], problems: [UserPartLibraryProblem]) {
        let keys: [URLResourceKey] = [.creationDateKey]
        guard let urls = try? FileManager.default.contentsOfDirectory(
            at: directory, includingPropertiesForKeys: keys) else { return ([], []) }
        var files: [UserPartLibraryFile] = []
        var problems: [UserPartLibraryProblem] = []
        let sorted = urls.filter { $0.pathExtension == PartFileType.fileExtension }
            .sorted { $0.lastPathComponent < $1.lastPathComponent }
        for url in sorted {
            guard let data = try? Data(contentsOf: url) else {
                problems.append(UserPartLibraryProblem(url: url, reason: "The file could not be read."))
                continue
            }
            guard let text = String(data: data, encoding: .utf8) else {
                problems.append(UserPartLibraryProblem(url: url, reason: "The file is not UTF-8 text."))
                continue
            }
            guard let uuid = (try? PartFileObject(text: text))?.uuid else {
                problems.append(UserPartLibraryProblem(url: url, reason: "The file is not a part file with a uuid."))
                continue
            }
            let added = (try? url.resourceValues(forKeys: Set(keys)).creationDate) ?? Date()
            files.append(UserPartLibraryFile(url: url, uuid: uuid, text: text, added: added))
        }
        return (files, problems)
    }

    func url(forUUID uuid: String) -> URL {
        directory.appendingPathComponent(uuid).appendingPathExtension(PartFileType.fileExtension)
    }

    /// Writes the part's file, creating the folder when it is missing.
    func write(uuid: String, text: String) throws {
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        try Data(text.utf8).write(to: url(forUUID: uuid), options: .atomic)
    }

    /// Deletes the part's file; a file that is not there is not an error.
    func remove(uuid: String) throws {
        try removeFile(at: url(forUUID: uuid))
    }

    /// Deletes any file in the folder (a problem entry).
    func removeFile(at url: URL) throws {
        do {
            try FileManager.default.removeItem(at: url)
        } catch let error as NSError where error.domain == NSCocoaErrorDomain && error.code == NSFileNoSuchFileError {
            return
        }
    }
}
