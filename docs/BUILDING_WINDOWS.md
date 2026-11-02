# Building CodeShelf on Windows 11 / 10

This guide explains how to configure the build environment, compile, test, and package CodeShelf natively on Windows using the modern MSYS2 UCRT64 toolchain or Microsoft Visual C++ (MSVC).

---

## 1. Prerequisites (MSYS2 UCRT64 Environment)

The recommended toolchain for Windows is **MSYS2 UCRT64**, which produces clean, native Windows binaries linked against the Universal C Runtime (`ucrtbase.dll`).

### 1.1. Install MSYS2
1. Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal and install the required compiler, Qt 6, SQLite 3, CMake, and Ninja packages:
   ```bash
   pacman -Syu
   pacman -S --needed \
       mingw-w64-ucrt-x86_64-gcc \
       mingw-w64-ucrt-x86_64-cmake \
       mingw-w64-ucrt-x86_64-ninja \
       mingw-w64-ucrt-x86_64-qt6-base \
       mingw-w64-ucrt-x86_64-sqlite3 \
       git
   ```

### 1.2. Ensure PATH Configuration
In PowerShell or Windows Command Prompt, add the UCRT64 binary directory to your PATH:
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
```

---

## 2. Configuring and Building with CMake Presets

CodeShelf includes predefined CMake presets in `CMakePresets.json`.

### 2.1. Debug Build
```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
```
The debug executable is produced at `build/windows-debug/codeshelf.exe`.

### 2.2. Release Build (Optimized)
```powershell
cmake --preset windows-release
cmake --build --preset windows-release
```
The release executable is produced at `build/windows-release/codeshelf.exe`.

---

## 3. Running Automated Tests

To execute the test suite (10 unit & integration test suites):
```powershell
# Set offscreen platform plugin for automated/headless test runs
$env:QT_QPA_PLATFORM = "offscreen"

# Run tests
.\build\windows-release\tests\codeshelf_tests.exe
```

Expected output:
```text
Totals: 10 passed, 0 failed, 0 skipped
```

---

## 4. Packaging a Portable Release

To create a standalone portable ZIP distribution:
1. Create a distribution folder:
   ```powershell
   New-Item -ItemType Directory -Path dist\codeshelf-windows-x64 -Force
   Copy-Item build\windows-release\codeshelf.exe dist\codeshelf-windows-x64\
   ```
2. Run `windeployqt` to collect Qt runtime DLLs and plugins:
   ```powershell
   windeployqt --release --no-translations --no-opengl-sw --no-compiler-runtime dist\codeshelf-windows-x64\codeshelf.exe
   ```
3. Copy GCC/UCRT runtime libraries:
   ```powershell
   Copy-Item C:\msys64\ucrt64\bin\libsqlite3-0.dll dist\codeshelf-windows-x64\
   Copy-Item C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll dist\codeshelf-windows-x64\
   Copy-Item C:\msys64\ucrt64\bin\libstdc++-6.dll dist\codeshelf-windows-x64\
   Copy-Item C:\msys64\ucrt64\bin\libwinpthread-1.dll dist\codeshelf-windows-x64\
   ```
4. Create ZIP package:
   ```powershell
   Compress-Archive -Path dist\codeshelf-windows-x64\* -DestinationPath dist\codeshelf-v0.1.0-windows-x64.zip
   ```
