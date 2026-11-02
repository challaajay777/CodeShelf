# CodeShelf Troubleshooting & FAQ

This guide provides troubleshooting solutions for common issues encountered during development, installation, or everyday usage.

---

## 1. Runtime & Launch Issues

### 1.1. Missing Runtime DLLs on Windows
**Symptom:** Launching `codeshelf.exe` displays an error such as:
`The code execution cannot proceed because libsqlite3-0.dll (or Qt6Core.dll) was not found.`

**Cause:** The application was run outside an environment where the compiler and Qt runtime DLLs are in the system `PATH`, or without running `windeployqt`.

**Resolution:**
- Use the official portable ZIP release in `dist/codeshelf-v0.1.0-windows-x64.zip`, which contains all bundled dependencies.
- If running from source, add your MSYS2 UCRT64 `bin` folder to your PATH:
  ```powershell
  $env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
  ```

---

## 2. Database & Storage Issues

### 2.1. `database is locked` / `busy timeout`
**Symptom:** UI displays `SQLite error: database is locked`.

**Cause:** Another process (such as an external SQLite GUI tool, backup software, or antivirus scanner) has placed an exclusive file lock on `codeshelf.db` or `codeshelf.db-wal`.

**Resolution:**
- Close any external database viewing utilities (e.g., DB Browser for SQLite).
- Ensure cloud sync folders (e.g., OneDrive, Dropbox) are not syncing the live database directory. CodeShelf is designed for local directory storage; use JSON exports or the Backup tool for cloud syncing.

### 2.2. Search Index Out of Sync
**Symptom:** Recent snippets do not appear in search results, or search yields inconsistent matches.

**Cause:** FTS5 virtual table was detached or suffered a non-atomic update.

**Resolution:**
- Open **Tools -> Maintenance...** in the menu bar.
- Click **Rebuild Full-Text Index**. This executes:
  ```sql
  INSERT INTO snippets_fts(snippets_fts) VALUES('rebuild');
  ```

### 2.3. Database Integrity Check Failure
**Symptom:** Maintenance dialog reports errors on `PRAGMA integrity_check;`.

**Resolution:**
1. Do not overwrite the file.
2. Locate the automatic safety backup in the data directory (`codeshelf_safety_backup_*.db`).
3. Restore the safety backup or export active snippets to JSON and re-import into a fresh database.

---

## 3. UI & Display Issues

### 3.1. High-DPI Scaling Blur or Microscopic UI
**Symptom:** Text appears blurry or icons and fonts are too small on 4K/retina monitors.

**Resolution:**
CodeShelf relies on Qt 6 native automatic High-DPI scaling. On Windows or Linux, ensure the following environment variables are set if your desktop environment does not supply scaling factors:
```bash
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_ENABLE_HIGHDPI_SCALING=1
```

### 3.2. Automated Tests Hang During Headless Execution
**Symptom:** Running `codeshelf_tests` hangs indefinitely.

**Cause:** A modal dialog (`QMessageBox::exec()`) was triggered without offscreen platform detection.

**Resolution:**
Always set `QT_QPA_PLATFORM=offscreen` before running tests in CI or scripts:
```powershell
$env:QT_QPA_PLATFORM = "offscreen"
.\build\windows-release\tests\codeshelf_tests.exe
```
CodeShelf's UI components explicitly inspect `QGuiApplication::platformName()` and bypass modal blocking dialogs when running offscreen.
