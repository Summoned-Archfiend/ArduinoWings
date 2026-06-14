# Design Changes & Review Notes

Working notes from reviewing [PLAN.md](PLAN.md). Prioritised for the deadline.
Severity: 🔴 resolve before wiring the motor · 🟠 address before powered tests · 🟡 verify / nice-to-have.

## Summary Table

| # | Severity | Area | Issue | Suggested change |
|---|----------|------|-------|------------------|
| 1 | 🔴 | Mechanism | A single pull-cable can only **open** the wing. Motor reverse just slackens the cable — nothing actively closes it. | Add a **return element** (spring/elastic/gravity bias) so the wing self-closes when the cable goes slack, OR a **second antagonistic cable**. Decide before wiring motor reverse. See [Closing the wing](#closing-the-wing). |
| 2 | 🟠 | Speed | 20T GT2 drum ≈ 12.7 mm dia → ~40 mm circumference. At 6 RPM = **~4 mm/s** cable travel. ~80–120 mm of pull ⇒ **20–30 s** to deploy. | Accept slow dramatic reveal, or fit a **larger-diameter drum** (faster; worm gear has torque to spare). Measure real travel during the string test first. |
| 3 | 🟠 | Power | Shared 12 V rail: motor stall/startup spikes sag the line and **reset the Arduino** on shared VIN. | Add **470–1000 µF electrolytic across BTS7960 B+/B-**; ideally power Arduino from a small **5 V buck** off 12 V instead of the linear VIN reg. Keep common ground. |
| 4 | 🟡 | Firmware/pins | Both RPWM and LPWM need PWM (direction alternates). | Confirm **D5/D6 are PWM-capable on Nano R4** (D3/5/6/9/10/11 on the UNO R4 RA4M1). `analogWrite` is 8-bit (0–255), so `128` is correct. |
| 5 | 🟡 | Firmware | `delay()` blocks button/limit-switch reads. | Use **non-blocking `millis()`** state machine throughout (already done in `WingSim/WingSim.ino`). |
| 6 | 🟡 | Code cleanup | `Blink.ino:16` `digitalWrite(ledPin, OUTPUT)` writes HIGH (`OUTPUT == 1`) — stray line. | Harmless; `WingSim` supersedes Blink. Delete Blink or ignore. |
| 7 | 🟢 | Process | Highest mechanical risk (cable attachment point + travel) is untested. | Do the **string pull-test (PLAN Step 6) now, in parallel** with electronics. It answers #1 and #2 for free. |

## Closing the wing

**This is the new concern and the most important one.** The plan opens the wing by winding a
cable onto a drum. A cable can only **pull**, so:

- Running the motor **forward** → winds cable → **opens** the wing. ✅
- Running the motor **reverse** → unwinds cable → **slackens** it. The wing does **not** close on its
  own — there is no force pushing it shut. ❌

So the `CLOSING` state in the control logic needs a real closing force. Options, roughly in
order of "easiest for the deadline":

| Option | How it works | Pros | Cons |
|--------|--------------|------|------|
| **A. Return spring / elastic** (recommended) | Spring or bungee biases the wing **closed**. Motor pulls cable to open against it; reverse the motor and the spring pulls the wing shut as cable slackens. | Simple, cheap, one cable, fewer parts. Worm gear still holds the open position. | Spring force must be tuned: strong enough to close reliably, weak enough that the motor can still open it. |
| **B. Gravity bias** | Mount geometry so the wing's natural resting position is closed; cable lifts it open. | No extra parts. | Only works if the closed position is genuinely "downhill"; unreliable while the wearer moves. |
| **C. Second antagonistic cable** | One cable opens, a second cable (own drum / opposite wrap) closes. | Fully powered both directions; precise. | Double the cable/pulley/routing work — risky under deadline. |

**Important interaction with the worm gear:** the worm drive is **self-locking** — it holds the wing
wherever the motor stopped, with no power. Great for *holding open*, but it means a spring (Option A)
**cannot** back-drive the gearbox to close the wing on its own. To close, the **motor must run in
reverse** to pay out cable while the spring takes up the slack. So closing is still motor-commanded;
the spring just supplies the closing force the cable can't.

**Recommendation:** Go with **Option A (return spring)** + powered reverse to release. Validate it
during the string test: with a spring/elastic fitted, pull the string to open, then release slowly and
confirm the wing closes smoothly on its own. If it does, the firmware's `CLOSING` state = run motor
reverse until the closed limit switch trips.

## Firmware status

- `WingSim/WingSim.ino` — button-driven `CLOSED→OPENING→OPEN→CLOSING` state machine, LED stands in for
  the motor. Non-blocking, debounced. Swap LED writes → BTS7960 calls and the `TRAVEL_MS` timeouts →
  limit-switch reads to make it the real controller.

## Open questions / to confirm during build

- [ ] Measure actual cable travel (mm) needed to fully open the wing (string test).
- [ ] Confirm a return spring can close the wing without stopping the motor opening it.
- [ ] Confirm D5/D6 PWM on the Nano R4.
- [ ] Pick drum diameter once travel + acceptable open time are known.
