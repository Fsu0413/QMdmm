# AGENTS.md

Guidance for AI agents working on this repository — coding standards only.
Project management (backlog / feedback / decisions) lives in a separate ledger
repository and is intentionally not covered here.

## Reference documentation

Read these before touching code:

- [README](README.md) — game rules, build/run instructions, protocol overview.
- [doc/architecture.md](doc/architecture.md) — how the modules
  (`QMdmmCore` / `QMdmmNetworking` / `QMdmmGui` / `QMdmmServer`) fit together.
- [doc/getting-started.md](doc/getting-started.md) — building, running a
  server, playing a game end-to-end, client API overview.

## Coding standards

### Memory management

Do not use `QScopedPointer` — it is deprecated in Qt. Use `std::unique_ptr`
instead. (Already applied in existing code; see commits `dddd2a4` / `2e00107`.)

### Source files are pure ASCII

- Comments and identifiers must contain no Chinese (or any other non-ASCII)
  characters.
- User-visible strings must go through i18n: `tr()` in C++ / `qsTr()` in QML,
  with translations in `QMdmmGui/translations/*.ts`. Never hardcode Chinese
  (or any natural-language) strings in code.

### Code formatting

- C++: run `clang-format -i` (config: `.clang-format`) on files you touched.
- QML: run `qmlformat -i` (config: `.qmlformat.ini`); note a known indentation
  bug — check the resulting diff afterwards.
