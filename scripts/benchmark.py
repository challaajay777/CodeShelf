#!/usr/bin/env python3
"""
CodeShelf Performance Benchmark Suite
Executes real, verifiable database and search operations against CodeShelf
datasets (100, 1,000, and 10,000 snippets) to record precise performance metrics.
"""

import sys
import os
import time
import json
import sqlite3
import hashlib
from pathlib import Path

def setup_database(db_path: Path):
    if db_path.exists():
        db_path.unlink()

    conn = sqlite3.connect(str(db_path))
    cur = conn.cursor()
    cur.execute("PRAGMA foreign_keys = ON;")
    cur.execute("PRAGMA journal_mode = WAL;")
    cur.execute("PRAGMA synchronous = NORMAL;")

    # Schema V1
    cur.executescript("""
        CREATE TABLE schema_metadata (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );

        CREATE TABLE collections (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            sort_order INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );

        CREATE TABLE tags (
            id TEXT PRIMARY KEY,
            normalized_name TEXT NOT NULL UNIQUE,
            display_name TEXT NOT NULL,
            created_at TEXT NOT NULL
        );

        CREATE TABLE snippets (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            body TEXT NOT NULL,
            description TEXT,
            language TEXT NOT NULL,
            snippet_type TEXT NOT NULL,
            collection_id TEXT REFERENCES collections(id) ON DELETE SET NULL,
            favorite INTEGER NOT NULL DEFAULT 0,
            pinned INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            deleted_at TEXT,
            content_hash TEXT NOT NULL,
            source_url TEXT
        );

        CREATE TABLE snippet_tags (
            snippet_id TEXT NOT NULL REFERENCES snippets(id) ON DELETE CASCADE,
            tag_id TEXT NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
            PRIMARY KEY (snippet_id, tag_id)
        );

        CREATE TABLE snippet_revisions (
            id TEXT PRIMARY KEY,
            snippet_id TEXT NOT NULL REFERENCES snippets(id) ON DELETE CASCADE,
            revision_number INTEGER NOT NULL,
            title TEXT NOT NULL,
            body TEXT NOT NULL,
            description TEXT,
            language TEXT NOT NULL,
            changed_at TEXT NOT NULL,
            change_note TEXT
        );

        CREATE TABLE templates (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            body TEXT NOT NULL,
            language TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );

        CREATE TABLE smart_views (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            query_string TEXT NOT NULL,
            sort_order INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL
        );

        CREATE INDEX idx_snippets_collection ON snippets(collection_id);
        CREATE INDEX idx_snippets_language ON snippets(language);
        CREATE INDEX idx_snippets_favorite ON snippets(favorite);
        CREATE INDEX idx_snippets_pinned ON snippets(pinned);
        CREATE INDEX idx_snippets_deleted_at ON snippets(deleted_at);
        CREATE INDEX idx_snippets_updated_at ON snippets(updated_at);
        CREATE INDEX idx_snippets_content_hash ON snippets(content_hash);
        CREATE INDEX idx_snippet_tags_tag ON snippet_tags(tag_id);
        CREATE INDEX idx_revisions_snippet ON snippet_revisions(snippet_id, revision_number);

        CREATE VIRTUAL TABLE snippets_fts USING fts5(
            snippet_id UNINDEXED,
            title,
            description,
            body,
            tags,
            tokenize = 'porter unicode61'
        );

        PRAGMA user_version = 1;
    """)
    conn.commit()
    return conn

def compute_hash(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()

def run_benchmark_for_file(json_file: Path, db_dir: Path):
    with open(json_file, "r", encoding="utf-8") as f:
        data = json.load(f)

    snippets = data.get("snippets", [])
    collections = data.get("collections", [])
    count = len(snippets)
    db_path = db_dir / f"benchmark_{count}.db"

    conn = setup_database(db_path)
    cur = conn.cursor()

    # 1. Measure Batch Insertion
    t0 = time.perf_counter()
    with conn:
        for col in collections:
            cur.execute(
                "INSERT INTO collections (id, name, description, sort_order, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?)",
                (col["id"], col["name"], col.get("description", ""), col.get("sort_order", 0),
                 "2026-01-01T00:00:00.000Z", "2026-01-01T00:00:00.000Z")
            )

        tag_map = {}
        for s in snippets:
            for t in s.get("tags", []):
                norm = t.lower()
                if norm not in tag_map:
                    tag_id = f"tag_{norm}"
                    cur.execute(
                        "INSERT OR IGNORE INTO tags (id, normalized_name, display_name, created_at) VALUES (?, ?, ?, ?)",
                        (tag_id, norm, t, "2026-01-01T00:00:00.000Z")
                    )
                    tag_map[norm] = tag_id

            h = compute_hash(s["body"])
            cur.execute("""
                INSERT INTO snippets (id, title, body, description, language, snippet_type, collection_id,
                                      favorite, pinned, created_at, updated_at, content_hash, source_url)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (s["id"], s["title"], s["body"], s["description"], s["language"], s["snippet_type"],
                  s.get("collection_id"), 1 if s.get("favorite") else 0, 1 if s.get("pinned") else 0,
                  s["created_at"], s["updated_at"], h, s.get("source_url", "")))

            for t in s.get("tags", []):
                cur.execute("INSERT OR IGNORE INTO snippet_tags (snippet_id, tag_id) VALUES (?, ?)",
                            (s["id"], tag_map[t.lower()]))

            # FTS insert
            tag_str = " ".join(s.get("tags", []))
            cur.execute("""
                INSERT INTO snippets_fts (snippet_id, title, description, body, tags)
                VALUES (?, ?, ?, ?, ?)
            """, (s["id"], s["title"], s["description"], s["body"], tag_str))

    t_insert = (time.perf_counter() - t0) * 1000.0

    # Flush WAL to get true file size
    cur.execute("PRAGMA wal_checkpoint(TRUNCATE);")
    file_size_kb = db_path.stat().st_size / 1024.0

    # 2. Measure Load All Active
    t0 = time.perf_counter()
    cur.execute("SELECT id, title, language, snippet_type, favorite, pinned, updated_at FROM snippets WHERE deleted_at IS NULL ORDER BY updated_at DESC")
    all_rows = cur.fetchall()
    t_load_all = (time.perf_counter() - t0) * 1000.0

    # 3. Measure FTS5 Search (single term)
    iterations = 50
    t0 = time.perf_counter()
    for _ in range(iterations):
        cur.execute("SELECT snippet_id FROM snippets_fts WHERE snippets_fts MATCH 'concurrency' LIMIT 50")
        _ = cur.fetchall()
    t_fts_single = ((time.perf_counter() - t0) / iterations) * 1000.0

    # 4. Measure FTS5 Exact Phrase Search
    t0 = time.perf_counter()
    for _ in range(iterations):
        cur.execute("SELECT snippet_id FROM snippets_fts WHERE snippets_fts MATCH '\"Worker pool started\"' LIMIT 50")
        _ = cur.fetchall()
    t_fts_phrase = ((time.perf_counter() - t0) / iterations) * 1000.0

    # 5. Measure Structured Query (Language + Type + Favorite)
    t0 = time.perf_counter()
    for _ in range(iterations):
        cur.execute("SELECT id FROM snippets WHERE deleted_at IS NULL AND language = 'C++' AND snippet_type = 'code' AND favorite = 1 ORDER BY updated_at DESC")
        _ = cur.fetchall()
    t_structured = ((time.perf_counter() - t0) / iterations) * 1000.0

    # 6. Fallback LIKE Query
    t0 = time.perf_counter()
    fallback_iters = 10 if count >= 10000 else iterations
    for _ in range(fallback_iters):
        cur.execute("SELECT id FROM snippets WHERE deleted_at IS NULL AND (title LIKE '%Worker%' OR body LIKE '%Worker%') LIMIT 50")
        _ = cur.fetchall()
    t_like = ((time.perf_counter() - t0) / fallback_iters) * 1000.0

    conn.close()

    return {
        "count": count,
        "insert_time_ms": t_insert,
        "file_size_kb": file_size_kb,
        "load_all_ms": t_load_all,
        "fts_single_ms": t_fts_single,
        "fts_phrase_ms": t_fts_phrase,
        "structured_query_ms": t_structured,
        "like_query_ms": t_like
    }

def main():
    root = Path(__file__).resolve().parent.parent
    fixtures_dir = root / "fixtures"
    db_dir = root / "build" / "benchmarks"
    db_dir.mkdir(parents=True, exist_ok=True)

    results = []
    print("=" * 70)
    print("CodeShelf Benchmarking Suite (Offline SQLite 3 + FTS5)")
    print("=" * 70)

    for count in [100, 1000, 10000]:
        json_file = fixtures_dir / f"snippets_{count}.json"
        if not json_file.exists():
            print(f"Skipping {json_file}: file does not exist.")
            continue
        print(f"Benchmarking with {count} snippets dataset...")
        res = run_benchmark_for_file(json_file, db_dir)
        results.append(res)
        print(f"  Count: {res['count']} snippets")
        print(f"  Batch Insert + FTS Indexing: {res['insert_time_ms']:.2f} ms")
        print(f"  Database File Size:          {res['file_size_kb']:.1f} KB ({res['file_size_kb']/1024:.2f} MB)")
        print(f"  Full Table Scan (Load All):  {res['load_all_ms']:.2f} ms")
        print(f"  FTS5 Single Term Search:     {res['fts_single_ms']:.3f} ms")
        print(f"  FTS5 Exact Phrase Search:    {res['fts_phrase_ms']:.3f} ms")
        print(f"  Indexed Structured Query:    {res['structured_query_ms']:.3f} ms")
        print(f"  Fallback LIKE Scan:          {res['like_query_ms']:.3f} ms")
        print("-" * 70)

    # Output JSON summary
    summary_path = db_dir / "benchmark_results.json"
    with open(summary_path, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2)
    print(f"Saved benchmark summary to {summary_path}")

if __name__ == "__main__":
    main()
