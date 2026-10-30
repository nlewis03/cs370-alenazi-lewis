# Fridge Door & Temperature Monitor - Team Rules

## Commands

- Build: `make` (builds both `door_sensor` and `temp_sensor` from the top-level Makefile; `make door_sensor` or `make temp_sensor` builds one)
- Clean: `make clean`
- Run: `./door_sensor /dev/gpiochip0 17` (chip/line are placeholders — match actual Pi wiring); temp sensor run command TBD once `temp.c` is written
- Dependencies (door sensor): `sudo apt install -y gcc make pkg-config libgpiod-dev gpiod`, then confirm `pkg-config --modversion libgpiod` reports 2.x
- Dependencies (temp sensor, DS18B20/1-Wire): enable the `w1-gpio` overlay (`/boot/config.txt` or `/boot/firmware/config.txt`) and load `w1-gpio`/`w1-therm` kernel modules; no extra library needed — reads come from `/sys/bus/w1/devices/<id>/w1_slave`
- Test: TODO — no automated test harness yet; see Evaluation Plan in DESIGN.md Section 6 for the manual fault-injection/timing tests to run before considering something done. For temp.c specifically, confirm it returns a plausible reading and handles a disconnected sensor without crashing before committing.
- `Makefile.door` is retired (commented out, kept for reference) — don't build from it anymore

## Hard Constraints

- No dependency on Claude Code or any online AI service at runtime — the device must run fully locally on the Pi.
- Storage is an append-only binary log; every record carries a timestamp, sensor type, length, and checksum. Never overwrite or silently drop prior valid records.
- On startup, validate the log and truncate only a trailing incomplete record — never discard earlier valid data.
- A failure in one sensor process must not take down the others (door and temperature run as separate processes; see failure-mode table in DESIGN.md Section 3).
- Never show a stale reading as current — if a reading is unavailable, say so explicitly in the interface and the log.
- Don't pass off recorded/simulated data as live sensor output in testing or demos — label it clearly.
- Door sensor requires libgpiod **2.x** specifically (API differs from 1.x).
- Temp sensor is a DS18B20 over 1-Wire, not libgpiod/I2C — read it via the kernel 1-Wire driver (`/sys/bus/w1/devices/.../w1_slave`), don't add a GPIO-bit-banging or I2C implementation for it.
- Build with `-std=c17 -Wall -Wextra -Werror -D_DEFAULT_SOURCE` (per the top-level Makefile) — keep warnings-as-errors clean for any new C file.

## Ownership

- **Abdulrahman**: door sensor program (`src/capture/door_sensor.c`), its build/instructions, larger share of the design doc.
- **Nailah**: hardware purchase/assembly, temperature sensor program (`src/capture/temp.c`), storage layer, supervisor, initial CLI/interface, M3 demo video.
- **Shared**: interface refinement, final integration/testing.

Keep this in sync with DESIGN.md Section 7 (Ownership Map) when responsibilities shift.

## Style

- C17, gcc, flags per the top-level Makefile (`-Wall -Wextra -Werror`) — treat warnings as build failures.
- Match existing code's comment density and naming rather than introducing a new convention per file.
- Debounce-style logic (e.g., door sensor's "three matching readings before reporting a change") should stay explicit and commented — these thresholds are load-bearing, not arbitrary.
- One component per directory under `src/` (capture / store / supervisor / interface) — keep new files in the matching one.
- DS18B20 requires roughly 750ms per conversion cycle at default 12-bit resolution, don't poll faster than this or reads will return stale/incomplete data

## Workflow

- Record AI-assisted decisions in each person's own `PROMPTLOG.md` under their individual folder (`abdulrahman/`, `nailah/`) — required per DESIGN.md Section 8 (AI-Use Plan).
- Keep DESIGN.md updated as architecture/failure-mode/ownership decisions change; don't let it drift from the actual code.
- Verify recovery paths against the failure-mode table in DESIGN.md Section 3 by actually triggering the failure (e.g., `kill -9` a sensor process and confirm the supervisor restarts it), not just by reasoning about it.
- Branch before committing to `main` unless told otherwise; ask before folding unrelated working-tree changes (deletions, generated files) into a commit.
- Don't commit `.DS_Store` or other OS/editor cruft.

## Open Items

- `temp_sensor` target builds against an empty `src/capture/temp.c` until that file has real code — expected to fail to link until then.
- `TEMP_LDLIBS` in the Makefile is correctly empty — DS18B20 is read via sysfs (`/sys/bus/w1/devices`), not a library. Leave it empty unless that approach changes.
- No test harness yet — formalize DESIGN.md Section 6's manual tests into scripts as components are built.
