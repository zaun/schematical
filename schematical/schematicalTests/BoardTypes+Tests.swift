@testable import schematical

/// The standard board types by name, for tests (the app knows them only
/// from `boards.json`).
extension BoardType {
    private static func named(_ id: String) -> BoardType {
        guard let type = BoardType(rawValue: id) else { fatalError("boards.json has no \(id)") }
        return type
    }

    static let isa8 = named("isa8")
    static let isa16 = named("isa16")
    static let pci = named("pci")
    static let pcie = named("pcie")
    static let m2 = named("m2")
    static let miniPCIe = named("miniPCIe")
    static let sodimm = named("sodimm")
    static let dimm = named("dimm")
    static let nes = named("nes")
    static let atari2600 = named("atari2600")
    static let atari5200 = named("atari5200")
    static let atari7800 = named("atari7800")
    static let atari8bit = named("atari8bit")
    static let atariST = named("atariST")
    static let jaguar = named("jaguar")
    static let lynx = named("lynx")
    static let coco = named("coco")
    static let piHat = named("piHat")
    static let arduinoShield = named("arduinoShield")
    static let featherWing = named("featherWing")
    static let pmod = named("pmod")
    static let mikroBus = named("mikroBus")
    static let beagleCape = named("beagleCape")
}
