import Foundation

/// The section titles a datasheet usually prints a step's information under
/// (plan-46-G § 1), most common first: what the step's Find in datasheet
/// button looks for. A kind's own steps get theirs from its guide (plan-46-L).
enum StepSearchTerms {
    static let identity = ["Ordering Information", "Package Option Addendum", "Orderable Part Number",
                           "Device Information"]
    static let pinout = ["Pin Configuration and Functions", "Pin Configuration", "Pin Functions",
                         "Pin Description", "Pin Assignments", "Terminal Functions", "Pinout"]
    static let supply = ["Recommended Operating Conditions", "Operating Conditions", "Absolute Maximum Ratings"]
    static let footprint = ["Land Pattern", "Recommended Land Pattern", "Example Board Layout",
                            "Recommended Footprint", "PCB Layout", "Mechanical Data", "Package Outline",
                            "Package Dimensions"]
}
