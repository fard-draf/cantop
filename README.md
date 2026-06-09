# cantop

A CAN bus diagnostic tool inspired by `htop`.

## Why

Working with CAN buses — N2K benches, J1939 machines — I kept running
into the same problem: `candump` gives you a raw stream, `cansniffer`
shows you PGN traffic, but neither answers the question I actually ask
first when something is wrong:

> *Which devices are on the bus right now, what are they sending, and
> how often?*

`cantop` is built around that question. Instead of a scrolling log, it
tracks each source address (SA) as a provider and shows its PGN
instances with counters and timestamps — closer to how `htop` presents
processes than how `tcpdump` presents packets.

The name is a nod to `htop`.

Runs on embedded Linux (tested on BeagleBone Black). No Python, no GUI, no runtime dependencies.

## Current state

Early development. The core pipeline works:

- raw CAN socket on `can0` (EFF / J1939 frames)
- per-SA provider tracking with PGN-level metrics
- event loop via `poll` + `signalfd` (SIGINT / SIGTERM / SIGHUP)
- output to stdout

Known limitations and planned work are tracked as `TODO` comments in
the source. This is not production software.

## Build

Requires: gcc, Linux with SocketCAN (`can0` interface up).

```sh
make
```

Sanitizers (`-fsanitize=address,undefined`) are enabled by default.
To build without them, edit `CFLAGS` in the Makefile.

## Usage

```sh
# bring up your CAN interface first
ip link set can0 up type can bitrate 250000

./cantop
```

Stop with `Ctrl-C` or `SIGTERM`. `SIGHUP` is reserved for future
runtime filter reload.

## Roadmap

- [ ] CLI flags: interface name, frame mode (SFF / EFF / both)
- [ ] Runtime filter via stdin (setsockopt on poll)
- [ ] ncurses display — scrollable provider/PGN table
- [ ] vcan0 replay for CI

## Related tools

- `candump` — raw frame logging
- `cansniffer` — byte-level change detection by ArbID
- `can-utils` — the standard CAN userspace toolkit
