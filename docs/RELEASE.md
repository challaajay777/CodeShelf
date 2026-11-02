# CodeShelf Release Engineering Guide

This guide outlines the standard operating procedure for preparing, validating, packaging, and publishing official releases of CodeShelf.

---

## 1. Versioning Scheme

CodeShelf adheres to [Semantic Versioning 2.0.0](https://semver.org/):
- **MAJOR**: Breaking changes to database schema or JSON backup format requiring manual migration steps.
- **MINOR**: New features, new search operators, UI enhancements, or non-breaking database schema additions.
- **PATCH**: Bug fixes, performance optimizations, and documentation updates.

Initial release version: `v0.1.0`.

---

## 2. Release Checklist

Every release must complete the following checklist before publication:

1. [ ] **Code Hygiene**:
   - No compiler warnings under `-Wall -Wextra -Wpedantic`.
   - Clang-format check passes.
   - Zero hardcoded machine paths or private developer credentials.
2. [ ] **Automated Tests**:
   - `codeshelf_tests` runs and passes 100% of test suites in both Debug and Release configurations.
   - Headless execution with `QT_QPA_PLATFORM=offscreen` succeeds.
3. [ ] **Performance Benchmarks**:
   - `python scripts/benchmark.py` passes without latency regressions.
4. [ ] **Documentation**:
   - `CHANGELOG.md` updated with release notes, additions, and fixes.
   - Version numbers synchronized in `CMakeLists.txt`, `JsonBackupHandler.cpp`, and `AboutDialog.cpp`.
5. [ ] **Packaging**:
   - Standalone portable Windows package created with `windeployqt` and verified on a clean machine without MSYS2 in PATH.
   - Linux package generated.
6. [ ] **Cryptographic Checksums**:
   - Generate SHA-256 checksums for all release archives.

---

## 3. Creating Release Artifacts

### 3.1. Windows Standalone Package
```powershell
# 1. Build release target
cmake --preset windows-release
cmake --build --preset windows-release

# 2. Package
$dist = "dist\codeshelf-windows-x64-v0.1.0"
New-Item -ItemType Directory -Path $dist -Force
Copy-Item build\windows-release\codeshelf.exe $dist\
windeployqt --release --no-translations --no-opengl-sw --no-compiler-runtime "$dist\codeshelf.exe"
Copy-Item C:\msys64\ucrt64\bin\libsqlite3-0.dll $dist\
Copy-Item C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll $dist\
Copy-Item C:\msys64\ucrt64\bin\libstdc++-6.dll $dist\
Copy-Item C:\msys64\ucrt64\bin\libwinpthread-1.dll $dist\

# 3. Archive & Hash
Compress-Archive -Path "$dist\*" -DestinationPath dist\codeshelf-v0.1.0-windows-x64.zip
Get-FileHash -Path dist\codeshelf-v0.1.0-windows-x64.zip -Algorithm SHA256
```

### 3.2. Linux Package
```bash
cmake --preset linux-release
cmake --build --preset linux-release
mkdir -p dist/codeshelf-linux-x64-v0.1.0
cp build/linux-release/codeshelf dist/codeshelf-linux-x64-v0.1.0/
tar -czvf dist/codeshelf-v0.1.0-linux-x64.tar.gz -C dist codeshelf-linux-x64-v0.1.0
sha256sum dist/codeshelf-v0.1.0-linux-x64.tar.gz
```

---

## 4. Git Tagging & GitHub Release

1. Commit changes:
   ```bash
   git commit -m "Release v0.1.0"
   ```
2. Tag release:
   ```bash
   git tag -a v0.1.0 -m "CodeShelf Version 0.1.0"
   ```
3. Push to remote:
   ```bash
   git push origin main --tags
   ```
4. Create GitHub release with binary attachments:
   ```bash
   gh release create v0.1.0 \
       dist/codeshelf-v0.1.0-windows-x64.zip \
       --title "CodeShelf v0.1.0" \
       --notes-file CHANGELOG.md
   ```
