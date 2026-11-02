# CodeShelf Testing Guide & Quality Assurance

This document details the quality assurance processes, test suite organization, automated execution, and fixture generation for CodeShelf.

---

## 1. Test Architecture

The automated test suite is implemented using the **Qt Test** (`QTest`) framework and compiled into a dedicated native test executable: `codeshelf_tests`.

The test suite covers:
1. **Core Domain Rules & Validation** (`TestValidation.cpp`): String boundaries, URL protocols, tag sanitization.
2. **Search Parser & Grammars** (`TestSearchParser.cpp`): Tokenization, quoted exact phrases, boolean operators (`lang:`, `tag:`, `collection:`, `is:fav`, `is:pin`, `is:trash`, `type:`).
3. **Hashing & Cryptography** (`TestContentHash.cpp`): SHA-256 exact content hash, whitespace & line-ending normalization hash.
4. **Diff Engine** (`TestDiff.cpp`): Myers / LCS line difference generation, insertion/deletion markers, context preservation.
5. **Duplicate Candidate Detection** (`TestDuplicate.cpp`): Exact duplicate identification and normalized similarity scanning.
6. **Import & Export Integrity** (`TestImportExport.cpp`): Round-trip JSON backup/restore, conflict strategies (Skip, Keep Both, Update), and safe Markdown code-fence escaping.
7. **Database Transactions & WAL Concurrency** (`TestDatabase.cpp`): Foreign key enforcement, atomic rollbacks on error, concurrent read stability.
8. **Schema Migrations** (`TestMigrations.cpp`): SQLite `PRAGMA user_version`, V1 schema instantiation, metadata triggers, FTS5 virtual table.
9. **Online Backup & Safety Restores** (`TestBackupRestore.cpp`): WAL-safe online backup API, automatic safety backup creation during restore.
10. **GUI Smoke Workflows** (`TestGui.cpp`): ViewModel item filtering, debounced search triggers, detail view field synchronization, unsaved dirty state detection.

---

## 2. Running Automated Tests

### 2.1. Headless Execution
To run tests without spawning visible OS windows, set the Qt Platform Abstraction (QPA) plugin to `offscreen`:

**On Windows (PowerShell):**
```powershell
$env:QT_QPA_PLATFORM = "offscreen"
.\build\windows-release\tests\codeshelf_tests.exe
```

**On Linux (Bash):**
```bash
export QT_QPA_PLATFORM=offscreen
./build/linux-release/tests/codeshelf_tests
```

### 2.2. Running a Specific Suite or Test Function
Pass the test name or function filter directly to the test executable:
```bash
./codeshelf_tests testSearchParserOperators
```

---

## 3. Performance & Fixture Benchmarking

To benchmark CodeShelf against large-scale datasets (100, 1,000, and 10,000 snippets):

1. **Generate Fixtures**:
   ```bash
   python scripts/generate_fixtures.py
   ```
   Generates JSON backups with diverse programming languages, tags, and collections into the `fixtures/` directory.

2. **Run Benchmark Suite**:
   ```bash
   python scripts/benchmark.py
   ```
   Measures batch insertion throughput, database file size on disk, full-table load times, FTS5 single-term search, FTS5 exact phrase search, and fallback LIKE queries.

---

## 4. Memory Safety & Sanitizers

To build with AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) on GCC / Clang:

```bash
cmake -B build/sanitizers -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build/sanitizers
./build/sanitizers/tests/codeshelf_tests
```
No memory leaks, dangling pointers, buffer overflows, or undefined behavior are tolerated.
