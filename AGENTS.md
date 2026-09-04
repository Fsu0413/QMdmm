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

### C-style variadic functions

- Do not write your own C-style variadic function (`(const char *fmt, ...)`)
  or use the `va_list` type anywhere in the codebase. Only *calling* a
  third-party variadic function (e.g. `qWarning("...%d", x)` or
  `QString::vasprintf`) is allowed.
- Variadic macros and template parameter packs are fine.
- For printf-style formatting with a dynamic argument list, use a variadic
  template + `QString::arg` chain instead. See `configError` in
  `QMdmmServer/src/config.cpp` for the canonical form.

### Use of `auto`

- Do not use `auto` when the concrete type can be written out explicitly —
  write `Protocol::PacketType`, `Client *`, `QList<LogicRunner *>`, etc.
- `auto` is allowed only in these cases:
  1. The type name is longer than 100 characters. Canonical example: the
     return type of `list2Set` in `qmdmmcore.cpp` is 126 chars —
     `QSet<typename std::remove_cv_t<typename std::iterator_traits<decltype(std::cbegin((const T &)std::declval<T>()))>::value_type>>`.
  2. The type is anonymous and cannot be named — a lambda or an anonymous
     struct/class.
  3. An upstream library's documentation explicitly requires `auto` — e.g.
     `qScopeGuard`, whose return type depends on the lambda closure type and
     cannot be spelled out.

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
