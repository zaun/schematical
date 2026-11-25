import AppKit
import WebKit

final class Probe: NSObject, WKNavigationDelegate {
    let web = WKWebView(frame: NSRect(x: 0, y: 0, width: 800, height: 600))
    var win: NSWindow!
    func start(_ url: String) {
        web.navigationDelegate = self
        win = NSWindow(contentRect: web.frame, styleMask: [.titled], backing: .buffered, defer: false)
        win.contentView = web
        win.orderBack(nil)
        web.load(URLRequest(url: URL(string: url)!))
        var n = 0
        Timer.scheduledTimer(withTimeInterval: 2, repeats: true) { t in
            n += 1
            self.web.evaluateJavaScript("document.title + ' | ' + location.href + ' | ' + document.body.innerText.slice(0,160).replace(/\\n/g,' / ')") { v, e in
                print("t+\(n*2)s:", v ?? e ?? "")
                if let s = v as? String, !s.hasPrefix("Just a moment") && n > 1 { NSApp.terminate(nil) }
                if n >= 12 { NSApp.terminate(nil) }
            }
        }
    }
}
let app = NSApplication.shared
app.setActivationPolicy(.accessory)
let p = Probe()
DispatchQueue.main.async { p.start(CommandLine.arguments[1]) }
app.run()
