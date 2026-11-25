# plan-60-A: Connections page and credential storage

Last updated: 2026-11-24
Overall Effort: large (plan-60 A+B+C; DigiKey sign-in through a web view, then BOM stock and price)
Effort: medium
Depends on: nothing

plan-60 adds a **Connections** page to Settings that holds the DigiKey API credentials, and a **Validate** button
on the Bill of Materials that looks up each line's stock and price at DigiKey. This sub-plan builds the page and
the storage behind it: a Client ID field, a Client Secret field, a status line, a Help popover with the steps for
getting an API key, and a `Connection` shape that GitHub, Mouser and others fill in later. It makes no network
call. plan-60-B adds the **Verify…** button and the sign-in; plan-60-C adds Validate.

A correct implementation of this sub-plan: Settings (⌘,) has a **Connections** page in the sidebar; typing a
Client ID and Secret keeps them across a restart (ID in UserDefaults, secret in the Keychain); **Help** shows the
steps to get an API key; the status line reads "Not filled in" or "Not verified".

References:

- `schematical/schematical/Preferences/PreferencesView.swift` (`PreferencesPage`, `PreferencesView`,
  `AutomationPreferencesView`: the page this one mirrors)
- `docs/agent-control.md` (Settings ▸ AI Agents section): the tone and shape the new `docs/connections.md` follows
- plan-60-B and plan-60-C (later letters)

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p60-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | not re-run at planning |
| No plan-60 code has landed | `ls schematical/schematical/Connections 2>&1` → `No such file or directory` | MET at planning |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue. If you stop, report the status of all prerequisites.

## 1. Goal

- Settings has a **Connections** page (sidebar icon `link`) with a **DigiKey** section: Client ID, Client Secret,
  status line, **Help**.
- The Client ID is stored in UserDefaults (`digikeyClientID`); the Secret is stored in the Keychain; neither is
  written to a document, a log or a test fixture.
- **Help** opens a popover with the steps for getting a DigiKey API key (text in § 4).
- A `Connection` type describes a service (id, title, icon, status, help steps) so a later service is one more
  value and one more section, with no change to the page.

### Non-goals (explicit constraints)

- No network call, no WebKit, no Verify button in this sub-plan (plan-60-B).
- No change to any document file format: credentials are app settings, never saved in a `.schematical` file.
- No change to the existing five Settings pages or to the `preferencesPage` `@AppStorage` key; the new case is
  added to the enum and old stored values still decode.

## 2. Current State

- `PreferencesPage` is a five-case enum (`partList, defaults, references, designRules, automation`) with `title`
  and `systemImage` switches (`Preferences/PreferencesView.swift:128-152`); `PreferencesView` is a
  `NavigationSplitView` sidebar whose detail switches on the page (`:156-182`). The window is 720×560.
- The stored page is `@AppStorage("preferencesPage")` (`PreferencesView.swift:157`); its raw values are the case
  names, so adding a case keeps old values valid.
- No Keychain code exists: `rg -l "SecItem|Keychain" schematical/schematical --glob '*.swift'` → no matches.
- The app is sandboxed with outgoing and incoming network allowed
  (`ENABLE_APP_SANDBOX = YES`, `ENABLE_OUTGOING_NETWORK_CONNECTIONS = YES`, `schematical.xcodeproj/project.pbxproj`
  lines 341-344 and 396-399). The Keychain needs no entitlement for the app's own items.
- Tests are XCTest classes under `schematicalTests/`, `@MainActor`, using `TestSupport` helpers.

### Measured populations

| What | Count | Command |
|---|---|---|
| Settings pages today | 5 | `rg -n '^    case partList' schematical/schematical/Preferences/PreferencesView.swift` → line 129, five names |
| Files in `Preferences/` | 4 | `ls schematical/schematical/Preferences \| wc -l` → 4 |
| Existing Keychain use | 0 | `rg -l "SecItem\|Keychain" schematical/schematical --glob '*.swift'` → no matches |

### Verified properties

- Adding an enum case to `PreferencesPage` needs a case in both `switch` statements and in the detail `switch`
  (read `PreferencesView.swift:133-151, 172-178`); the Swift compiler enforces all three.

## 3. Design Overview

Four small pieces, no layering risk:

1. `Connections/SecretStore.swift`: a `SecretStore` protocol (`get(_:)`, `set(_:for:)`, `remove(_:)`, keyed by
   an account string) with two implementations: `KeychainSecretStore` (generic-password items, service
   `<bundle id>.connections`) and `MemorySecretStore` (tests, and the type the views use in previews).
2. `Connections/Connection.swift`: `struct Connection` (id, title, symbol, help steps) and
   `enum ConnectionStatus { notFilledIn, notVerified, verified(Date) }` with its one-line `text`.
   `Connection.digikey` is the only value.
3. `Connections/DigiKeyCredentials.swift`: `@Observable @MainActor final class DigiKeyCredentials` with
   `clientID`, `clientSecret`, `status`, `clear()`; reads UserDefaults and a `SecretStore`; writes through on
   every change; a change to either field removes the stored login tokens (plan-60-B stores them under accounts
   `digikey.refreshToken` and the `digikeyVerifiedAt` default; the removal code lives here so B only adds
   writers).
4. `Preferences/ConnectionsPreferencesView.swift`: a `Form` (`.formStyle(.grouped)`) with one `Section` per
   connection: header (icon, title, status badge, **Help** button), `TextField` Client ID, `SecureField` Client
   Secret. The Help popover is `ConnectionHelpView` showing the steps as a numbered list plus a link button to
   `https://developer.digikey.com`.

Correctness risk is low: the only subtle point is that editing a credential must invalidate a login made with the
old one, which is why `DigiKeyCredentials` removes the tokens on change.

Rejected alternatives:

- Storing the secret in UserDefaults or `@AppStorage`: plain text in a readable plist. The Keychain is the
  platform place for a secret.
- A tab strip instead of a sidebar page: Settings is already a sidebar of pages; a "tab" would be a second
  navigation style.
- One generic key-value "connections" blob: each service has different fields; a typed `DigiKeyCredentials` is
  clearer. The shared part is `Connection` and `SecretStore`.

## 4. Detailed Design

### Help text (the exact steps shown)

1. "Go to developer.digikey.com and sign in with a DigiKey account (it is free)."
2. "Create an organization if you have none (Organizations ▸ Add Organization)."
3. "Open the organization and add a **Production App**. Enable the **Product Information** API."
4. "Set the OAuth callback (redirect) URL to exactly `https://localhost/schematical`."
5. "Save the app, open it, and copy its **Client ID** and **Client Secret** into this page."
6. "Click **Verify…** and sign in to DigiKey in the window that opens. Tick *Verify you are human* if asked."

Step 6 refers to the button plan-60-B adds; the Help text ships in A with the button's name already in it, and
the Help popover is the only place that mentions it until B lands.

### Status text

| Status | Text |
|---|---|
| `notFilledIn` (either field empty) | "Not filled in" |
| `notVerified` (both filled, no login) | "Not verified" |
| `verified(date)` | "Verified " + the date, medium date style |

## Compatibility / Format Impact

None. No document, catalog or export format changes. New UserDefaults key: `digikeyClientID`. New Keychain
account: `digikey.clientSecret`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work it describes.
> `- [~]` is partial; a moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's `Commit:` line when it
> lands. An unticked box means NOT DONE.

### Phase 1 — Secret store and credentials model

Lands alone: nothing uses it yet.

- [ ] `Connections/SecretStore.swift`: `SecretStore`, `KeychainSecretStore` (`SecItemAdd`/`SecItemCopyMatching`/
      `SecItemUpdate`/`SecItemDelete`, `kSecClassGenericPassword`, `kSecAttrService` = `<bundle id>.connections`,
      `kSecAttrAccount` = the key, `kSecAttrAccessibleWhenUnlocked`), `MemorySecretStore`.
- [ ] `Connections/Connection.swift`: `Connection`, `ConnectionStatus`, `Connection.digikey` with the six help
      steps from § 4.
- [ ] `Connections/DigiKeyCredentials.swift`: as § 3.3; `status` is `notFilledIn` when either field is empty,
      `verified(date)` when `digikeyVerifiedAt` and the `digikey.refreshToken` secret both exist, else
      `notVerified`.
- [ ] Tests `schematicalTests/ConnectionsTests.swift`: status for empty/ID only/both/both+token; editing the ID
      or the secret removes `digikey.refreshToken` and `digikeyVerifiedAt`; `MemorySecretStore` round trip;
      `KeychainSecretStore` round trip under a unique per-run service name, removed in `tearDown`.
- [ ] Add the new files to the Xcode project (`schematical.xcodeproj/project.pbxproj`).

Acceptance: the status rules and invalidation rule hold, and a real Keychain write/read/delete round trips in the
test host.
  Check: `XC test -only-testing:schematicalTests/ConnectionsTests` → `** TEST SUCCEEDED **` (est. 5 min, a build
  plus one class; `XC` = `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical
  -derivedDataPath /tmp/p60-dd CODE_SIGNING_ALLOWED=NO`).
Commit: —

### Phase 2 — The Connections page

- [ ] `Preferences/PreferencesView.swift`: add `case connections` to `PreferencesPage` (title "Connections",
      symbol `link`), place it last, and add `ConnectionsPreferencesView()` to the detail switch.
- [ ] `Preferences/ConnectionsPreferencesView.swift`: the form of § 3.4; the secret field is a `SecureField`;
      the status badge is a `Label` with a green check when verified and a secondary dot otherwise.
- [ ] `Preferences/ConnectionHelpView.swift`: the popover of § 3.4, steps as `Text` rows with inline code spans,
      a `Link` to `https://developer.digikey.com`.
- [ ] Tests (same class): `PreferencesPage.allCases.last == .connections`; every page has a non-empty title and
      symbol; the stored raw value `"partList"` still decodes.

Acceptance: Settings ▸ Connections shows the DigiKey section; typing and quitting/relaunching keeps the ID and
secret; Help opens and closes.
  Check: owner-visible pass in the real app, Debug build from `/tmp/p60-dd`: open Settings, pick Connections,
  type an ID and secret, quit, relaunch, see both kept; click Help (est. 10 min; no unit test sees a popover).
Commit: —

## Validation Plan

- Tests: `ConnectionsTests` (§ Phase 1 and 2).
- Coverage check: `ConnectionsTests` imports `@testable import schematical`, so the new files are in the test
  target's denominator.
- Runtime proof: the real-app pass in Phase 2.
- Doc sync: add `docs/connections.md` (the page, where each value is stored, the Help steps); add the page to the
  list of Settings pages anywhere the docs list them: `rg -n "Settings ▸" docs/*.md` shows `docs/agent-control.md`
  and `docs/file-format.md` mention pages by name; neither lists them all, so no edit there.
- Final gate (run once): `XC test -only-testing:schematicalTests/ConnectionsTests
  -only-testing:schematicalTests/DesignRuleSettingsTests` → `** TEST SUCCEEDED **` (est. 8 min; the second class
  is the existing Settings test so a Settings regression shows).

## Decisions

- The Connections page is a sidebar page, one section per service, not a tab — decided by the owner, 2026-11-24.
  (§ 3)
- The secret is stored in the Keychain; the ID in UserDefaults — author's call, no user-visible consequence. (§ 3)

## Corrections

(none yet)

## Summary

Low risk: a page, two stores and a status rule. Untouched: every document format, the other Settings pages, and
all networking, which plan-60-B owns.
