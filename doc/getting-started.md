# Getting Started

Build QMdmm and run a complete game end-to-end. The GUI cannot play a full game
yet, so the "play a game" part below uses the headless smoke test (bots) — the
same in-process server + clients the GUI's local-game mode is built on.

## Prerequisites

- CMake ≥ 3.19
- Qt ≥ 6.5 (Core / Network / WebSockets / Gui / Qml / Quick / Widgets /
  QuickWidgets)
- A C++20 compiler

## Build

```sh
qt-cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
ninja -C build
```

Binaries land in `build/build/bin/`.

## Run the server

```sh
./build/build/bin/QMdmmServer6
```

By default it listens for TCP on port 6366 and WebSocket on port 6367. It takes
a full set of command-line options (room size, damage and HP values, timeouts,
transport toggles, …); run `--help` to see the table.

## Run a client (GUI)

```sh
./build/build/bin/QMdmm6
```

Currently this shows the start menu only; playing a full game from the GUI is
still being wired up.

## Play a headless game (bots)

The smoke test spins up an in-process server plus two auto-driven clients and
plays a full game to completion, including a mid-game disconnect/reconnect:

```sh
ctest --test-dir build -R qmdmm_smoke --output-on-failure
```

## Run the full test suite

```sh
ctest --test-dir build --output-on-failure
```

## Drive a client yourself

To control a `Client` programmatically (your own bot or a custom frontend),
connect to its request signals and answer through its reply slots:

| Request signal | Reply slot |
|---|---|
| `requestRockPaperScissors` | `replyRockPaperScissors` |
| `requestActionOrder` | `replyActionOrder` |
| `requestAction` | `replyAction` |
| `requestUpgrade` | `replyUpgrade` |

`smoke/main.cpp` contains a complete, competent auto-player (buy a knife, slash
a co-located enemy, otherwise walk toward one, and spend every upgrade point)
you can copy from.
