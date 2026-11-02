# CodeShelf Backup, Restore & Disaster Recovery Guide

Data integrity is the central pillar of CodeShelf. This guide details how database backups and restores are executed safely without risking data corruption.

---

## 1. Backup Strategies

CodeShelf provides two distinct backup mechanisms:
1. **Physical Binary Database Backup (`.db`)**: Exact byte-for-byte replica of the SQLite database created via SQLite's Online Backup API.
2. **Logical Portable Backup (`.json`)**: Versioned `codeshelf-backup` JSON file, ideal for cross-platform migration, inspection, or selective import.

---

## 2. WAL-Safe Binary Backup

### 2.1. The Danger of Blind File Copying
In SQLite Write-Ahead Logging (WAL) mode, changes are written to the `-wal` companion file before being checkpointed into the main `.db` file. Blindly copying `codeshelf.db` with OS copy utilities (`cp` or `Copy-Item`) while the database is open results in:
- Incomplete data (missing changes residing in the WAL ring buffer).
- Torn pages if a write occurs mid-copy.
- Corruption if the accompanying `-wal` and `-shm` files are mismatched.

### 2.2. SQLite Online Backup API Implementation
CodeShelf uses the official SQLite Online Backup API (`sqlite3_backup_init`, `sqlite3_backup_step`, `sqlite3_backup_finish`).
- Locks are acquired cooperatively without blocking reading transactions.
- All pages from the main database and WAL buffer are seamlessly transferred into the target backup file.
- The resulting backup file is completely self-contained and does not require WAL or SHM companion files.

---

## 3. Safe Database Restore Procedure

Restoring an existing database must be guarded against accidental data loss. CodeShelf executes a multi-step safety protocol when restoring from a `.db` backup:

1. **Pre-Restore Validation**:
   - The candidate backup file is inspected using SQLite to verify its schema version and execute `PRAGMA integrity_check;`.
   - If corrupted or incompatible, the restore operation aborts immediately.

2. **Automatic Safety Backup**:
   - Before touching the active database, CodeShelf automatically generates a safety snapshot of the currently open database:
     `codeshelf_safety_backup_YYYYMMDD_HHMMSS_zzz.db`.
   - If anything fails during the restore process, the user can recover from this safety snapshot.

3. **Connection Teardown & File Swap**:
   - The active database connection is cleanly closed and memory caches are flushed to release all OS file locks (critical on Windows).
   - The existing database, `-wal`, and `-shm` files are replaced with the verified backup file.
   - The database connection is reopened, schema migrations are verified, and UI models are refreshed.

---

## 4. Disaster Recovery Scenarios

### Recovering from an Accidental Restore
If an unintended database restore was performed:
1. Navigate to the CodeShelf data directory:
   - **Windows:** `%LOCALAPPDATA%\CodeShelf\` (or configured database folder).
   - **Linux:** `~/.local/share/CodeShelf/`.
2. Locate the most recent file matching `safety_backup_*.db`.
3. Open CodeShelf and use **Tools -> Restore Database...**, selecting the safety backup file.
