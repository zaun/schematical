import AppKit
import WebKit

final class Probe: NSObject, WKNavigationDelegate {
    let web = WKWebView(frame: NSRect(x: 0, y: 0, width: 800, height: 600))
    var step = 0
    func start() {
        web.navigationDelegate = self
        let win = NSWindow(contentRect: web.frame, styleMask: [.titled], backing: .buffered, defer: false)
        win.contentView = web
        win.orderBack(nil)
        // 1) native URLSession token POST with bogus creds
        var req = URLRequest(url: URL(string: "https://api.digikey.com/v1/oauth2/token")!)
        req.httpMethod = "POST"
        req.setValue("application/x-www-form-urlencoded", forHTTPHeaderField: "Content-Type")
        req.httpBody = "client_id=bogus&client_secret=bogus&grant_type=client_credentials".data(using: .utf8)
        URLSession.shared.dataTask(with: req) { d, r, e in
            let h = r as? HTTPURLResponse
            print("URLSession token POST:", h?.statusCode ?? -1, h?.value(forHTTPHeaderField: "cf-mitigated") ?? "-", String(data: d ?? Data(), encoding: .utf8)?.prefix(120) ?? "")
            DispatchQueue.main.async { self.web.load(URLRequest(url: URL(string: "https://api.digikey.com/")!)) }
        }.resume()
    }
    func webView(_ w: WKWebView, didFinish n: WKNavigation!) {
        step += 1
        if step == 1 {
            w.evaluateJavaScript("document.title + ' | ' + location.href") { v, e in
                print("WK page 1:", v ?? e ?? "")
                let js = """
                fetch('/v1/oauth2/token',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'client_id=bogus&client_secret=bogus&grant_type=client_credentials'}).then(async r=>r.status+' '+(r.headers.get('cf-mitigated')||'-')+' '+(await r.text()).slice(0,120))
                """
                w.callAsyncJavaScript("return await (" + js + ")", arguments: [:], in: nil, in: .page) { res in
                    print("WK fetch token POST:", res)
                    let auth = "https://api.digikey.com/v1/oauth2/authorize?response_type=code&client_id=bogus&redirect_uri=https%3A%2F%2Flocalhost%2Fschematical&state=x"
                    w.load(URLRequest(url: URL(string: auth)!))
                }
            }
        } else if step == 2 {
            w.evaluateJavaScript("document.title + ' | ' + location.href + ' | ' + document.body.innerText.slice(0,200)") { v, e in
                print("WK authorize page:", v ?? e ?? "")
                NSApp.terminate(nil)
            }
        }
    }
    func webView(_ w: WKWebView, didFail n: WKNavigation!, withError e: Error) { print("fail", e); NSApp.terminate(nil) }
    func webView(_ w: WKWebView, didFailProvisionalNavigation n: WKNavigation!, withError e: Error) { print("failprov", e); NSApp.terminate(nil) }
}
let app = NSApplication.shared
app.setActivationPolicy(.accessory)
let p = Probe()
DispatchQueue.main.async { p.start() }
DispatchQueue.main.asyncAfter(deadline: .now() + 60) { print("timeout"); NSApp.terminate(nil) }
app.run()
