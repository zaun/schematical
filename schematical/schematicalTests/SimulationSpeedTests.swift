import XCTest
@testable import schematical

/// The Simulation › Speed submenu ticks the choice nearest the stored speed.
final class SimulationSpeedTests: XCTestCase {
    func testNearestChoice() {
        XCTAssertEqual(SimulationSpeed.choice(nearest: 1), 1)
        XCTAssertEqual(SimulationSpeed.choice(nearest: 1e-3), 1e-3)
        XCTAssertEqual(SimulationSpeed.choice(nearest: 0.0011), 1e-3)
        XCTAssertEqual(SimulationSpeed.choice(nearest: 5), 1)
        XCTAssertEqual(SimulationSpeed.choice(nearest: 1e-9), 1e-6)
        XCTAssertEqual(SimulationSpeed.choices.count, 7)
    }
}
