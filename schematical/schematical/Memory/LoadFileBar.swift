//
//  LoadFileBar.swift
//  schematical
//
//  The "Load file on start" bar of a part that takes a load file: an
//  EEPROM's contents, in its memory window, or a simulated
//  microcontroller's firmware, in its processor window (plan-62-H).
//

import SwiftUI

/// A part's "Load file on start" setting. It can change only while the
/// simulation is stopped.
struct LoadFileBar: View {
    let file: MemoryFile?
    let isRunning: Bool
    let choose: () -> Void
    let clear: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Text("Load file on start:")
            Text(file?.name ?? "None — starts erased (FF)")
                .foregroundStyle(file == nil ? .secondary : .primary)
                .lineLimit(1)
                .truncationMode(.middle)
                .help(file?.url.path ?? "The chip starts with every byte FF")
            Spacer()
            Button("Choose…", action: choose)
                .disabled(isRunning)
                .help(isRunning ? "Stop the simulation to change the load file"
                                : "Choose a binary or Intel HEX file to load at every start")
            Button("Clear", action: clear)
                .disabled(isRunning || file == nil)
                .help(isRunning ? "Stop the simulation to change the load file"
                                : "Start erased (every byte FF)")
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }
}
