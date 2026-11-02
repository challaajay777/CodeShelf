# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-11-02

### Added
- **Core Snippet Management**: Full CRUD operations for snippets with title, body, description, language, type, collection, tags, source URL, timestamps, favorite, and pinned flags.
- **Language Support**: 25+ language identifiers (C++, Python, Rust, Go, TypeScript, JavaScript, SQL, Shell, etc.) with safe Plain Text fallback.
- **SQLite Engine**: Embedded SQLite 3.53.4 database with WAL mode, foreign key enforcement, transactional rollback, and versioned migrations.
- **Instant Search with FTS5**: Debounced full-text search engine with Porter stemming and Unicode 61 tokenization, plus fallback LIKE queries.
- **Rich Operator Parser**: Structured query operators (`lang:`, `tag:`, `collection:`, `type:`, `is:favorite`, `is:pinned`, `is:trash`, and quoted exact phrases).
- **Revision History & Diffs**: Automatic revision snapshots on edits, line-oriented Myers LCS diff comparison with addition/deletion markers, and historical restore.
- **Duplicate Scanner**: Exact duplicate detection via SHA-256 and normalized whitespace comparison, with side-by-side inspection.
- **Template Engine**: Code snippets with token placeholders (`{{variable}}`) with instant parameter filling and insertion.
- **Trash & Reversibility**: Soft-delete pattern with Trash bin, item recovery, and confirmed permanent purge.
- **Online Backup & Safety Restores**: WAL-safe SQLite Online Backup API and pre-restore automatic safety snapshots.
- **Database Maintenance**: Native diagnostics for `PRAGMA integrity_check;`, WAL compaction (`VACUUM`), and FTS5 rebuilds.
- **Import / Export**: Versioned `codeshelf-backup` JSON import/export with 3-way conflict resolution (Skip, Keep Both, Update), and Markdown export with dynamic code-fence escaping.
- **Native Desktop UI**: Minimal, conventional Qt 6 Widgets interface adhering to native system styling without cyberpunk/neon effects or web views.
- **Automated Test Suite**: 10 comprehensive Qt Test suites covering all layers.
