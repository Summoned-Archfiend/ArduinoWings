# Circuit Connections

Wiring reference for the motorised hip-wing. Pins match the firmware in
[WingControl/WingControl.ino](WingControl/WingControl.ino). See [PLAN.md](PLAN.md)
for the design and [DESIGN_CHANGES.md](DESIGN_CHANGES.md) for review notes.

Convention: all switches use the Nano's internal pull-ups (`INPUT_PULLUP`), so
**not pressed = HIGH, pressed/triggered = LOW**. No external resistors needed.

---

## Arduino Nano R4 — pin map

| Nano pin | Connects to | Direction | Notes |
|----------|-------------|-----------|-------|
| **D2** | Button leg 1 | input | open/close; quick tap. Other button leg → GND |
| **D3** | Open limit switch **NO** | input | triggered when wing fully OPEN. Switch **C** → GND |
| **D4** | Closed limit switch **NO** | input | triggered when wing fully CLOSED. Switch **C** → GND |
| **D5** `~` | BTS7960 **RPWM** | output (PWM) | motor speed, one direction |
| **D6** `~` | BTS7960 **LPWM** | output (PWM) | motor speed, other direction |
| **D7** | BTS7960 **R_EN** | output | enable (on/off) |
| **D8** | BTS7960 **L_EN** | output | enable (on/off) |
| **5V** | BTS7960 **VCC** | power out | logic supply for the driver |
| **GND** | Common ground node | ground | see ground section below |
| **VIN** | (final build only) 12 V via fuse + switch | power in | **leave disconnected while on USB** |
| **USB** | PC / power bank | power in | how it's powered during bench testing |

Unused on Nano: everything else.

---

## Button (D2)

| Button | Connects to |
|--------|-------------|
| leg 1 | D2 |
| leg 2 | GND (common node) |

Polarity doesn't matter (plain switch). 2-leg button: either leg either side.

---

## Limit switches — two per end, paralleled (D3 open, D4 closed)

Both wings share the single driver, so each wing gets its own limit switches and
the matching pair is **wired in parallel** onto one pin. Either switch triggering
pulls the pin LOW = "that end reached". 3-terminal microswitches — use **C and
NO only**, leave NC unconnected.

| Switch | Terminal | Connects to |
|--------|----------|-------------|
| Left open limit | C / NO | GND node / **D3** |
| Right open limit | C / NO | GND node / **D3** (parallel with left) |
| Left closed limit | C / NO | GND node / **D4** |
| Right closed limit | C / NO | GND node / **D4** (parallel with left) |

So D3 = both open switches in parallel, D4 = both closed switches in parallel.

> ⚠️ If a switch reads TRIGGERED at rest, it's on the **NC** terminal — move the
> wire to **NO**. (NC would need the firmware logic inverted.)
>
> Because sensing is shared, the wings must be **mechanically matched** so they
> reach each end together — the motor stops as soon as the *first* wing trips its
> switch.

---

## BTS7960 motor driver (single driver, both motors)

Both wings run from **one** BTS7960. The two motors are wired in **parallel** on
M+/M- and always move together — to the firmware this is just one motor (D5–D8
unchanged).

### Logic side (to Arduino — thin signal wire, ~22 AWG)

| BTS7960 | Connects to |
|---------|-------------|
| RPWM | Nano D5 |
| LPWM | Nano D6 |
| R_EN | Nano D7 |
| L_EN | Nano D8 |
| VCC  | Nano 5V |
| GND  | Common ground node |
| R_IS | not connected |
| L_IS | not connected |

### Power side (to battery + motor — thick wire, ~18–20 AWG)

| BTS7960 | Connects to |
|---------|-------------|
| B+ | Battery + (via fuse + main switch in final build) |
| B- | Common ground node (heavy path — see below) |
| M+ | **Both** motors, lead A (one motor reversed — see note) |
| M- | **Both** motors, lead B (one motor reversed — see note) |

Both motors connect in parallel:

```
M+ ──┬── Left motor ──┬── M-
     └── Right motor ─┘   (Right motor's two leads REVERSED vs Left)
```

> **Mirror the direction:** the wings are mirror images, so reverse **one motor's
> two leads** so both wings open the same way on "forward". Confirm on the first
> low-speed test — if one wing goes the wrong way, swap that motor's two wires.
>
> If *both* wings move the wrong way: swap **M+ and M-** at the driver (easiest),
> or swap the `motorOpen()`/`motorClose()` bodies in code.

---

## Common ground (star point)

All grounds must meet at **one** node. Do **not** stack many wires on a single
Nano GND pin — run one wire from Nano GND to the node, then join the rest there
(soldered splice + heatshrink, or a bridged terminal block).

Lands on the common ground node:

- Nano **GND**
- Button leg 2
- Both open limit switches **C** (×2)
- Both closed limit switches **C** (×2)
- BTS7960 logic **GND**
- BTS7960 **B-**  ← heavy wire (carries both motors' current)
- Battery **–**   ← heavy wire (final build)

> Keep the motor-current grounds (B-, battery –) on **thick wire** joining the
> node directly; don't route motor current through the thin signal grounds.

---

## Power

| Stage | How powered |
|-------|-------------|
| **Bench testing now** | Arduino via **USB**. Motors via 12 V (battery/bench supply) into BTS7960 B+/B- only. **Do not also feed VIN.** |
| **Final untethered build** | 12 V battery → **fuse (~5 A)** → **main switch** → splits to: Arduino **VIN** *and* BTS7960 **B+**. Common ground throughout. |

> USB powers the Arduino logic ONLY — it cannot drive the motors. Motor power
> always comes from the 12 V supply through the BTS7960.
>
> **Two motors:** combined stall is ~4 A, so size the battery for **~4–5 A peak**
> and fuse at **~5 A**. Fit a **470–1000 µF electrolytic across B+/B-** to smooth
> the start-up/stall inrush (prevents the battery BMS tripping).

---

## Wire gauge quick reference

| Use | Gauge |
|-----|-------|
| Signal (button, limits, D5–D8, 5V, logic GND) | ~22 AWG stranded |
| Motor power (B+, B-, M+, M-, battery) | ~18–20 AWG stranded |

Use **stranded** (not solid) everywhere that flexes, and add strain relief
(zip tie / hot glue) near connections so movement doesn't crack joints.

---

## Boot behaviour reminder

At power-on the firmware infers position from the limit switches:
- Closed limit (D4) pressed → boots to **CLOSED**
- Open limit (D3) pressed → boots to **OPEN**
- Neither pressed → boots to **FAULT** (hold a limit + hold button ~1.5 s to recover)

So in normal use, power on with the wings folded (D4 engaged) and it starts in
CLOSED automatically.
