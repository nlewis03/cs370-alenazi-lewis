#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

// temp.c - DS18B20 temperature sensor capture (1-Wire via sysfs)
//
// Planning skeleton only - no implementation yet. Fill in each section
// below. Mirror door_sensor.c's overall shape (signal handling, timestamped
// printing, main loop) where it makes sense, but the sensor I/O itself is
// sysfs file reads, not gpiod calls, so most of door_sensor.c's gpiod setup
// doesn't carry over.

// ---------------------------------------------------------------------------
// 1. Includes (started above - add to this list as needed)
// ---------------------------------------------------------------------------
// - signal.h for graceful shutdown on SIGINT/SIGTERM, same as door_sensor.c.
// - No gpiod.h here - this sensor is not a gpiod line.

// ---------------------------------------------------------------------------
// 2. Global shutdown flag
// ---------------------------------------------------------------------------
// - volatile sig_atomic_t running flag + signal handler, same pattern as
//   door_sensor.c's stop_program(). Keep the loop interruptible.

// ---------------------------------------------------------------------------
// 3. Locate the DS18B20 device directory
// ---------------------------------------------------------------------------
// - The kernel exposes each 1-Wire device under /sys/bus/w1/devices/<id>,
//   where <id> starts with "28-" for DS18B20. The id is not known ahead of
//   time (depends on the chip's factory-burned address), so:
//     - Either accept the device id/path as a command-line argument
//       (consistent with door_sensor.c taking chip+line as argv), or
//     - Scan /sys/bus/w1/devices/ for an entry starting with "28-" and use
//       the first match.
// - Build the full path to that device's `w1_slave` file.
// - Decide what argv should look like for this program (document it in a
//   usage message, same style as door_sensor.c's "Usage: %s ..." message).

// ---------------------------------------------------------------------------
// 4. Read one raw sample from w1_slave
// ---------------------------------------------------------------------------
// - Open the w1_slave file fresh each read (triggers a new conversion read
//   from the kernel driver - don't cache the fd across reads).
// - The file contains two lines, e.g.:
//      4d 01 4b 46 7f ff 0c 10 74 : crc=74 YES
//     4d 01 4b 46 7f ff 0c 10 74 t=19812
// - Parse the second line's `t=<value>` - value is millidegrees C
//   (divide by 1000.0 for Celsius).
// - Check the first line's trailing "YES"/"NO" - this is the CRC check the
//   driver already did. Treat "NO" as a failed/garbage read, same severity
//   as a missing t= value.
// - Think about what counts as "read returns error/timeout" per the
//   failure-mode table (file open fails, CRC says NO, t= missing/unparsable,
//   or the read taking unexpectedly long) - that's the condition the retry
//   logic in step 6 responds to.

// ---------------------------------------------------------------------------
// 5. Plausibility check
// ---------------------------------------------------------------------------
// - DS18B20 range is roughly -55C to +125C; a fridge interior should be in
//   a much narrower band. Decide on a sane bounds check here so a garbage
//   parse doesn't get reported as a real reading.
// - This is a separate check from the CRC check in step 4 - CRC says the
//   bytes weren't corrupted in transit, plausibility says the *value*
//   makes sense for this application.

// ---------------------------------------------------------------------------
// 6. Retry-once-then-unavailable logic
// ---------------------------------------------------------------------------
// - Per DESIGN.md's failure-mode table: on a failed/implausible read, retry
//   once, then mark the reading unavailable rather than reporting a stale
//   or bogus value.
// - Decide how "unavailable" gets communicated downstream (a sentinel
//   value? a separate status field? whatever the storage layer/interface
//   expects to receive from this process).

// ---------------------------------------------------------------------------
// 7. print_reading() - timestamped output
// ---------------------------------------------------------------------------
// - Same shape as door_sensor.c's print_reading(): get current time,
//   strftime it, print "<timestamp> Temp: <value>C" (or "<timestamp> Temp:
//   unavailable" per step 6). Flush stdout like door_sensor.c does.

// ---------------------------------------------------------------------------
// 8. main()
// ---------------------------------------------------------------------------
// - Parse argv (see step 3's decision on what args this program takes).
// - Install signal handlers (step 2).
// - Resolve the w1_slave path once at startup (step 3) - or re-resolve on
//   each read if you want to tolerate the sensor being reconnected; decide
//   and note which.
// - Loop while running:
//     - Read + validate a sample (steps 4-6).
//     - print_reading() the result.
//     - Sleep before the next read.
//       IMPORTANT: DS18B20 needs ~750ms per conversion at default 12-bit
//       resolution (see CLAUDE.md Style section) - don't poll faster than
//       that or you'll read stale/incomplete conversions. DESIGN.md's
//       ~2-second interval already respects this, just don't shrink it
//       without re-checking the conversion-time math.
// - On shutdown (running == 0), print a "Temp sensor stopped" message,
//   same style as door_sensor.c, and return 0.

// ---------------------------------------------------------------------------
// 9. Before considering this done
// ---------------------------------------------------------------------------
// - Confirm it returns a plausible reading on a working sensor.
// - Confirm it handles a disconnected/misread sensor without crashing
//   (this is the fault-injection test from DESIGN.md Section 6 and the
//   Test line in CLAUDE.md's Commands section - actually unplug it and
//   watch what happens, don't just reason about it).
// - Match door_sensor.c's build flags (-std=c17 -Wall -Wextra -Werror
//   -D_DEFAULT_SOURCE) - this file needs to compile clean under those too.
