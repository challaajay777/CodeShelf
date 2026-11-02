# Building CodeShelf on Linux

This guide covers building, testing, and packaging CodeShelf on Linux (Ubuntu 22.04 / 24.04, Debian 12, Fedora, and Arch Linux).

---

## 1. System Requirements & Dependencies

CodeShelf requires a C++20 compliant compiler (GCC 12+ or Clang 15+), Qt 6.4+, SQLite 3 (with FTS5 support), and CMake 3.22+.

### 1.1. Ubuntu / Debian
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    qt6-base-dev \
    qt6-base-dev-tools \
    libsqlite3-dev \
    git
```

### 1.2. Fedora / Red Hat Enterprise Linux
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    qt6-qtbase-devel \
    sqlite-devel \
    git
```

### 1.3. Arch Linux
```bash
sudo pacman -Syu --needed \
    base-devel \
    cmake \
    ninja \
    qt6-base \
    sqlite \
    git
```

---

## 2. Building with CMake Presets

CodeShelf includes Linux presets in `CMakePresets.json`.

### 2.1. Debug Configuration
```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
```
Binary output: `build/linux-debug/codeshelf`

### 2.2. Release Configuration
```bash
cmake --preset linux-release
cmake --build --preset linux-release
```
Binary output: `build/linux-release/codeshelf`

---

## 3. Running Automated Tests

Run the full Qt test suite headlessly using the `offscreen` QPA platform plugin:

```bash
export QT_QPA_PLATFORM=offscreen
./build/linux-release/tests/codeshelf_tests
```

All 10 test suites should report clean passes:
```text
Totals: 10 passed, 0 failed, 0 skipped
```

---

## 4. Packaging for Linux

### 4.1. Standalone Tarball
```bash
mkdir -p dist/codeshelf-linux-x64
cp build/linux-release/codeshelf dist/codeshelf-linux-x64/
cp resources/icon.svg dist/codeshelf-linux-x64/
cp LICENSE dist/codeshelf-linux-x64/
cp README.md dist/codeshelf-linux-x64/
tar -czvf dist/codeshelf-v0.1.0-linux-x64.tar.gz -C dist codeshelf-linux-x64
```

### 4.2. Desktop Integration
Create `codeshelf.desktop`:
```ini
[Desktop Entry]
Name=CodeShelf
Comment=Offline Native Desktop Code Snippet Organizer
Exec=codeshelf
Icon=codeshelf
Terminal=false
Type=Application
Categories=Development;Utility;TextEditor;
```
Install to `~/.local/share/applications/` and icon to `~/.local/share/icons/hicolor/scalable/apps/codeshelf.svg`.
