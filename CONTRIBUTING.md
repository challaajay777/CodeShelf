# Contributing to CodeShelf

Thank you for your interest in contributing to CodeShelf! We welcome pull requests, bug reports, and discussions aligned with our core product principles.

---

## 1. Core Architectural Invariants

Before proposing any changes, please note our immutable product constraints:
1. **Offline-First & Privacy First**: CodeShelf will never contain telemetry, cloud accounts, remote analytics, or background network calls.
2. **Inert Text Only**: Snippets must never be compiled, evaluated, or executed by the application.
3. **No Web Frontends**: The application is strictly a native C++20 / Qt 6 Widgets desktop utility. No Electron, Tauri, WebViews, or embedded local web servers.
4. **Native Minimal Styling**: Restrained, native OS widgets. No AI-styled glow, neon highlights, glassmorphism, or oversized cards.
5. **Data Safety**: All modifications must be wrapped in atomic transactions with schema versioning.

---

## 2. Development Setup

### 2.1. Prerequisites
- **Compiler**: Modern C++20 compiler (GCC 12+, Clang 15+, or MSVC 2022).
- **Framework**: Qt 6.4+ (Widgets, Core, Gui).
- **Database**: SQLite 3 (compiled with FTS5).
- **Build System**: CMake 3.22+ and Ninja.

### 2.2. Building & Testing
Follow the instructions in:
- [docs/BUILDING_WINDOWS.md](docs/BUILDING_WINDOWS.md)
- [docs/BUILDING_LINUX.md](docs/BUILDING_LINUX.md)

Always ensure that 100% of the automated test suite passes before submitting:
```bash
export QT_QPA_PLATFORM=offscreen
./build/release/tests/codeshelf_tests
```

---

## 3. Code Style & Conventions

- **Standard**: C++20.
- **Indentation**: 4 spaces. No tabs in C++ files.
- **Naming**:
  - Types / Classes / Structs: `PascalCase` (e.g., `SnippetRepository`).
  - Methods / Functions: `camelCase` (e.g., `getById()`, `updateSnippet()`).
  - Private Member Variables: `m_camelCase` (e.g., `m_database`, `m_titleEdit`).
  - Constants: `PascalCase` or `kCamelCase`.
- **Memory Safety**: Prefer RAII, smart pointers (`std::unique_ptr`, `std::shared_ptr`), and Qt parent-child ownership. Avoid raw pointer ownership or manual `new`/`delete`.
- **String Handling**: Use `QStringLiteral` for constant strings and UTF-8 encoding throughout.

---

## 4. Submitting a Pull Request

1. Fork the repository and create a feature branch from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
2. Commit changes with clear, descriptive commit messages.
3. Run both the unit test suite and the performance benchmark suite:
   ```bash
   python scripts/benchmark.py
   ```
4. Push your branch and open a Pull Request against `main`.
