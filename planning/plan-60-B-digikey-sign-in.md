# plan-60-B: DigiKey sign-in through a web view

Last updated: 2026-11-24
Effort: large
Depends on: plan-60-A

plan-60-B adds the **Verify…** button. It opens a window with a WebKit view on `api.digikey.com`; the user passes
Cloudflare's human check, signs in to DigiKey, and the app takes the authorization code from the redirect, trades
it for an access token and a refresh token, and stores the refresh token in the Keychain. It also builds the one
transport every later DigiKey call uses: `fetch` run inside that web view's `api.digikey.com` page.

A correct implementation: with a Client ID and Secret filled in, **Verify…** shows the sign-in window; after the
user signs in the window closes and the status reads "Verified <date>"; quitting and relaunching keeps it
verified; a later call gets a fresh access token without a window.

References:

- DigiKey 3-legged OAuth: <https://developer.digikey.com/tutorials-and-resources/oauth-20-3-legged-flow> (the
  site answers automated fetches with 403; the flow below is taken from its search excerpts and from the
  token-endpoint probes in § 2)
- `scripts/digikey.js` (the Safari transport this replaces inside the app; its header comment on Cloudflare)
- plan-60-A (`DigiKeyCredentials`, `SecretStore`, `Connection`)

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-60-A is complete | `rg -l "DigiKeyCredentials" schematical/schematical/Connections` → one file; `rg -n "Commit: —" planning/plan-60-A-connections-page.md` → no matches | NOT MET at planning |
| The owner has a DigiKey Production App with callback `https://localhost/schematical` and Product Information enabled | owner states it before Phase 3's real-window pass | owner's |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue. If you stop, report the status of all prerequisites.

## 1. Goal

- **Verify…** on Settings ▸ Connections ▸ DigiKey signs the user in and leaves the connection "Verified".
- `DigiKeyConnection.accessToken()` returns a valid access token, refreshing silently when it has expired.
- `DigiKeyConnection.send(_:)` makes one DigiKey API request and returns status, headers and body, handling
  Cloudflare's check by showing the window when it appears.
- **Disconnect** removes the login.

### Non-goals (explicit constraints)

- No BOM change, no product lookup (plan-60-C).
- No client-credentials (2-legged) flow and no `URLSession` call to `api.digikey.com`: both are blocked (§ 2).
- No new entitlement, no Apple Events, no Safari scripting.

## 2. Current State

Measured at planning, 2026-11-24, macOS 15.8.1 (`sw_vers -productVersion`):

| Probe | Result |
|---|---|
| `URLSession` POST `https://api.digikey.com/v1/oauth2/token` with fake credentials | `403`, header `cf-mitigated: challenge`, body "Just a moment..." (`planning/plan-60-data/cloudflare-probe-urlsession-and-fetch.swift`, first output line) |
| A bare `WKWebView` loads `https://api.digikey.com/` and the authorize URL; title polled every 2 s for 24 s | stays "Just a moment..." the whole time, and `fetch` of the token route from that page returns the same `403` challenge (`cloudflare-probe-urlsession-and-fetch.swift`, lines 2-4; `cloudflare-probe.swift <url>` → 12 lines, all "Just a moment...") |
| Same with `applicationNameForUserAgent = "Version/18.6 Safari/605.1.15"`, 16 s | still "Just a moment..." |
| Snapshot of that web view at 12 s | Cloudflare's **"Verify you are human"** checkbox (`planning/plan-60-data/cloudflare-check.png`) |
| `scripts/digikey.js` (sync XHR inside a Safari window on `api.digikey.com`) | works: `GET /products/v4/search/SN74HC00N/productdetails` → `200`, `x-ratelimit-limit: 1000` |
| Token endpoint, form `grant_type=authorization_code&code=bogus&client_id&client_secret&redirect_uri` | `400 {"ErrorMessage":"Invalid authCode","ErrorDetails":"Get new authCode",...}` |
| Token endpoint, form `grant_type=refresh_token&refresh_token=bogus&client_id&client_secret` | `401 {"ErrorMessage":"Invalid RefreshToken","ErrorDetails":"Ensure the refresh_token parameter has a valid refresh token",...}` |
| Token endpoint, `grant_type=client_credentials` with a wrong secret | `401 {"ErrorMessage":"invalid_client","ErrorDetails":"Client credentials are invalid",...}` |

So the token endpoint takes the form fields below and reports failures as `{ErrorMessage, ErrorDetails}` JSON. A
person can pass the check in a visible web view; a script cannot. The Safari route proves a browser that has
passed the check reaches the API.

DigiKey's documented flow (excerpts of the page above): authorize at `https://api.digikey.com/v1/oauth2/authorize`
with `response_type=code`, `client_id`, `redirect_uri` (HTTPS, exactly the registered value, trailing slash
included) and `state`; the code lasts one minute; exchange at `https://api.digikey.com/v1/oauth2/token`; API
calls send `Authorization: Bearer <token>` and `X-DIGIKEY-Client-Id`; each refresh returns a new refresh token and
the old one stops working. Lifetimes differ between DigiKey's pages (access 30 minutes; refresh "does not expire"
on the tutorial, 90 days on the FAQ), so the code reads `expires_in` from each response and treats a failed
refresh as "not verified".

Existing code: `Datasheet/MarkdownDatasheetWindow.swift:85-186` is the app's only `WKWebView` use (a
`WKNavigationDelegate` that decides navigation policy, `NSViewRepresentable` wrapper); this plan mirrors it.
`Datasheet/DatasheetLoader.swift:35` is the only `URLSession` use; it is unrelated.

### Measured populations

| What | Count | Command |
|---|---|---|
| `WKWebView` uses in the app | 1 file | `rg -l "WKWebView" schematical/schematical --glob '*.swift'` → `MarkdownDatasheetWindow.swift` |
| DigiKey daily quota left when probed | 23 of 1000 | `x-ratelimit-remaining` header on the last probe call (shared with other sessions) |

### Verified properties

- A web view with no visible window still runs JavaScript: the first probe ran `fetch` through
  `callAsyncJavaScript` in a never-shown window and got its response (the `403`).
- `ENABLE_OUTGOING_NETWORK_CONNECTIONS = YES` (project.pbxproj lines 344, 399): WebKit in the app may connect.

## 3. Design Overview

One `@MainActor final class DigiKeyConnection` owns one `WKWebView` that uses `WKWebsiteDataStore.default()`, so
the Cloudflare clearance cookie and the DigiKey login cookies persist across launches. The web view lives in a
window that is shown only when the user has something to do (the check, the login).

Pieces, in layering order:

1. `OAuthRedirect` (pure): builds the authorize URL; parses a redirect URL into `.code(String)` /
   `.denied(String)`; checks `state`. Fully unit tested.
2. `DigiKeyTokens` (pure): decodes a token response (`access_token`, `refresh_token`, `expires_in`; `expires_in`
   is read as a number or a numeric string) and an error response (`ErrorMessage`, `ErrorDetails`).
3. `DigiKeyTransport` protocol: `func send(_ request: DigiKeyRequest) async throws -> DigiKeyResponse` and
   `func showHumanCheck() async throws` (shows the window on the `api.digikey.com` page and returns when its
   title is no longer "Just a moment..."; throws `challengeTimedOut` after 120 s).
   `WebViewTransport` implements it with `callAsyncJavaScript`; tests use a fake.
4. `DigiKeyConnection`: sign-in, `accessToken()`, `send(_:)`, `disconnect()`.
5. The Verify… and Disconnect buttons and the status line in `ConnectionsPreferencesView`.

Where the risk concentrates: the web view and the human check (pieces 3 and 4). It cannot be exercised by a unit
test, so Phase 3 ends with a real-window pass by the owner. The pure pieces carry the logic that tests can reach.

Flows:

- **Verify…**: (1) store nothing yet; show the window; load the authorize URL with a fresh `state` (a UUID); the
  user passes the check if shown and signs in. (2) `decidePolicyFor`: a navigation to `https://localhost/schematical`
  is cancelled and parsed. A `state` mismatch or a `denied` result shows an alert with the message. (3) Hide the
  window, load `https://api.digikey.com/` in the web view, wait until its title is not "Just a moment..." (the
  window is shown again if the check appears; 120 s limit then an error), then run `fetch` on that page: POST
  `/v1/oauth2/token`, `application/x-www-form-urlencoded`, fields `grant_type=authorization_code`, `code`,
  `client_id`, `client_secret`, `redirect_uri`. (4) Decode; save the refresh token to the `SecretStore` account
  `digikey.refreshToken` and `digikeyVerifiedAt` = now before anything else; keep the access token and its expiry
  in memory.
- **`accessToken()`**: returns the in-memory token when more than 60 s remain; else one shared in-flight refresh
  (`grant_type=refresh_token`, `refresh_token`, `client_id`, `client_secret`), saving the new refresh token
  before returning. A `401` or `400` from the refresh removes the refresh token and `digikeyVerifiedAt` and
  throws `DigiKeyError.notVerified`.
- **`send(_:)`**: on `403` with `cf-mitigated: challenge` the window is shown on the page until the title leaves
  "Just a moment...", then the request is made once more.
- **Disconnect**: remove the refresh token and `digikeyVerifiedAt`, clear the access token, and remove the
  web view's `digikey.com` website data records (cookies, local storage); the next Verify shows the check again.

Constants: redirect URI `https://localhost/schematical`; API base `https://api.digikey.com`; request headers
`X-DIGIKEY-Client-Id`, `X-DIGIKEY-Locale-Site: US`, `X-DIGIKEY-Locale-Language: en`,
`X-DIGIKEY-Locale-Currency: USD` (the headers `scripts/digikey.js:298-304` sends).

Rejected alternatives:

- `ASWebAuthenticationSession` (the system browser sheet): its `https` callback needs an associated-domains
  file on a domain we own, and the token POST afterwards still comes from native code that Cloudflare blocks.
- 2-legged client credentials through `URLSession`: measured `403` (§ 2).
- Driving Safari with Apple Events like `scripts/digikey.js`: needs the user to enable "Allow JavaScript from
  Apple Events" and an Apple Events entitlement; the owner's rule is platform-native tooling used properly.
- A localhost HTTP server for the redirect: DigiKey requires HTTPS and the web view's navigation delegate
  already sees the redirect URL.

## 4. Detailed Design

### Files (all under `schematical/schematical/Connections/`)

- `OAuthRedirect.swift`: `static func authorizeURL(clientID:state:) -> URL`;
  `static func parse(_ url: URL, expectedState: String) -> OAuthRedirectResult`.
- `DigiKeyTokens.swift`: `struct DigiKeyTokens { accessToken, refreshToken, expiresAt }`,
  `init(response: Data, now: Date) throws`, `struct DigiKeyAPIError { message, details }`.
- `DigiKeyTransport.swift`: `DigiKeyRequest { method, path, headers, body: Data?, form: [String: String]? }`,
  `DigiKeyResponse { status, headers: [String: String], body: Data }`, `WebViewTransport`.
- `DigiKeyConnection.swift`: the class above; `DigiKeyError` (`notVerified`, `challengeTimedOut`, `api(status,
  message)`, `rateLimited(resetAt: String?, retryAfter: Int?)`).
- `DigiKeyWindow.swift`: an `NSWindow` (titled "DigiKey", 520×720, `isReleasedWhenClosed = false`) hosting the
  web view; `show()`/`hide()`; closing it with the red button while a Verify is waiting cancels the Verify.

### `fetch` bridge

`callAsyncJavaScript` body (arguments passed as typed values, never string-built):
`const r = await fetch(path, {method, headers, body}); const h = {}; for (const [k,v] of r.headers) h[k] = v;
return {status: r.status, headers: h, body: await r.text()};`. The page's origin is `https://api.digikey.com`, so
`path` is relative. Bodies are `String`s: form requests are `URLSearchParams`-style percent-encoded in Swift.

### Verify button state

Enabled when both fields are non-empty and no Verify is running; while running the button reads "Verifying…"
with a `ProgressView`. After success the status line reads "Verified <date>" and the button label becomes
**Verify Again…**; **Disconnect** is shown beside it.

## Compatibility / Format Impact

None to documents. New Keychain account `digikey.refreshToken`, new UserDefaults key `digikeyVerifiedAt`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work it describes.
> `- [~]` is partial; a moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's `Commit:` line when it
> lands. An unticked box means NOT DONE.

### Phase 1 — OAuth parsing and token decoding

Lands alone: pure code with no caller.

- [ ] `Connections/OAuthRedirect.swift` and `Connections/DigiKeyTokens.swift` as § 4.
- [ ] Tests `schematicalTests/DigiKeyOAuthTests.swift`: the authorize URL has `response_type=code`, the
      percent-encoded redirect `https%3A%2F%2Flocalhost%2Fschematical`, the `state` and the `client_id`; a
      redirect `https://localhost/schematical?code=abc&state=S` parses to `.code("abc")`; a wrong `state` is
      `.denied`; `?error=access_denied&state=S` is `.denied("access_denied")`; a token response with `expires_in`
      as `1799` and as `"1799"` both decode to `now + 1799`; the three error bodies measured in § 2 decode to
      their `ErrorMessage`/`ErrorDetails`.
- [ ] Add the files to `project.pbxproj`.

Acceptance: every case above passes.
  Check: `XC test -only-testing:schematicalTests/DigiKeyOAuthTests` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 2 — Connection logic against a fake transport

- [ ] `Connections/DigiKeyTransport.swift` (protocol and value types) and `Connections/DigiKeyConnection.swift`
      with the transport and the `SecretStore` injected; `signIn(code:)`, `accessToken()`, `send(_:)`,
      `disconnect()` as § 3. The window and navigation are not in this phase: `signIn(code:)` takes the code.
- [ ] Tests `schematicalTests/DigiKeyConnectionTests.swift` with a fake transport and `MemorySecretStore`:
      `signIn(code:)` posts the five form fields and saves the refresh token and `digikeyVerifiedAt`; a second
      `accessToken()` call inside 60 s of expiry does not call the transport; after expiry it posts the refresh
      form and saves the new refresh token before returning; two concurrent expired `accessToken()` calls make
      one refresh request; a `401` refresh removes the stored tokens and throws `notVerified`; `send(_:)` on a
      `403` + `cf-mitigated: challenge` calls the transport's `showHumanCheck()` then retries once; a `429`
      throws `rateLimited` with `x-ratelimit-resettime` and `retry-after` copied from the headers.

Acceptance: every case above passes.
  Check: `XC test -only-testing:schematicalTests/DigiKeyConnectionTests` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 3 — Web view, sign-in window and the Verify button (largest blast radius last)

- [ ] `Connections/DigiKeyWindow.swift` and `WebViewTransport` (in `DigiKeyTransport.swift`): the web view on
      `WKWebsiteDataStore.default()`, `show()/hide()`, the `fetch` bridge of § 4, the title wait for "Just a
      moment...", the navigation delegate that cancels and parses the redirect.
- [ ] `DigiKeyConnection.verify()`: the Verify flow of § 3 using the window.
- [ ] `Preferences/ConnectionsPreferencesView.swift`: **Verify…** / **Verify Again…** and **Disconnect** buttons,
      the progress state, an alert showing a `DigiKeyError`'s message.
- [ ] Owner-driven real-window pass (the owner does this; the agent does not tick a human check): fill in the
      ID and secret, click **Verify…**, tick *Verify you are human*, sign in, see "Verified <date>"; quit and
      relaunch, still "Verified"; **Disconnect** returns to "Not verified". When driving the app, move the pointer
      to (0, 0) afterwards (AGENTS.md).

Acceptance: the owner's pass above succeeds on a Debug build, with no window shown on relaunch and no new
human check while the web view's cookies last.
  Check: owner-visible pass as above (est. 20 min; a window, a human check and a real login have no unit test).
Commit: —

## Validation Plan

- Tests: `DigiKeyOAuthTests`, `DigiKeyConnectionTests`, plus `ConnectionsTests` from plan-60-A.
- Coverage check: all three classes `@testable import schematical`; the new files are in the test target.
- Runtime proof: Phase 3's owner pass.
- Doc sync: `docs/connections.md` (from plan-60-A): add the sign-in flow, the Cloudflare check, what Disconnect
  removes, and the redirect URI the user must register.
- Final gate (run once): `XC test -only-testing:schematicalTests/DigiKeyOAuthTests
  -only-testing:schematicalTests/DigiKeyConnectionTests -only-testing:schematicalTests/ConnectionsTests` →
  `** TEST SUCCEEDED **` (est. 10 min).

## Decisions

- Sign in with DigiKey's 3-legged OAuth in a WebKit window, not client credentials — decided by the owner,
  2026-11-24. (§ 3)
- The button is **Verify…** and the status line says Verified — decided by the owner, 2026-11-24. (§ 4)
- The owner declined a Cloudflare probe before planning; the design assumes the check is passed once by the user
  in the sign-in window and its clearance persists in the web view's default data store, the behavior of the
  Safari route in `scripts/digikey.js`. Phase 3's owner pass is the first run that exercises it. — owner,
  2026-11-24. (§ 3)
- Redirect URI `https://localhost/schematical` — author's call: HTTPS is required and `localhost` needs no domain.
  (§ 3)

## Corrections

(none yet)

## Summary

The risk is the web view passing Cloudflare and the login round trip; the pure OAuth and token code and the
refresh logic are covered by unit tests against a fake transport. Untouched: the BOM, documents, entitlements.
