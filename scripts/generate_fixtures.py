#!/usr/bin/env python3
"""
CodeShelf Fixture Generator
Generates realistic, syntactically valid JSON backup fixtures for CodeShelf
containing snippets across diverse programming languages, tags, and collections.
"""

import json
import uuid
import random
import datetime
from pathlib import Path

LANGUAGES = [
    ("C++", "cpp", "int main(int argc, char** argv) {\n    std::cout << \"Hello from C++!\" << std::endl;\n    return 0;\n}"),
    ("Python", "py", "def process_data(items):\n    return [x * 2 for x in items if x % 2 == 0]"),
    ("Rust", "rs", "fn compute_hash(data: &[u8]) -> u64 {\n    let mut sum: u64 = 0;\n    for &b in data { sum = sum.wrapping_add(b as u64); }\n    sum\n}"),
    ("JavaScript", "js", "async function fetchUser(id) {\n    const res = await fetch(`/api/users/${id}`);\n    return res.json();\n}"),
    ("TypeScript", "ts", "interface UserConfig {\n    id: string;\n    active: boolean;\n    timeoutMs: number;\n}"),
    ("Go", "go", "package main\n\nimport \"fmt\"\n\nfunc main() {\n    fmt.Println(\"Worker pool started\")\n}"),
    ("SQL", "sql", "SELECT u.id, u.username, COUNT(s.id) as snippet_count\nFROM users u\nLEFT JOIN snippets s ON u.id = s.user_id\nGROUP BY u.id;"),
    ("Shell", "sh", "#!/usr/bin/env bash\nset -euo pipefail\necho \"Deployment finished at $(date)\""),
    ("Markdown", "md", "# Project Documentation\n\n## Overview\nThis is a sample technical documentation note with markdown formatting."),
    ("JSON", "json", "{\n  \"status\": \"healthy\",\n  \"uptime\": 86400,\n  \"version\": \"1.0.0\"\n}")
]

TYPES = ["code", "note", "command", "config", "sql", "pseudocode"]
TAG_POOL = ["algorithms", "concurrency", "networking", "database", "security", "ui", "file-io", "parsing", "math", "utilities", "debugging", "performance", "async", "testing", "patterns"]

COLLECTIONS = [
    {"id": str(uuid.uuid4()), "name": "Algorithms & Data Structures", "description": "Standard algorithms, sorting, graph traversal, and trees", "sort_order": 0},
    {"id": str(uuid.uuid4()), "name": "System Architecture", "description": "Low-level, OS abstractions, networking, and memory management", "sort_order": 1},
    {"id": str(uuid.uuid4()), "name": "Web Services & APIs", "description": "REST endpoints, serialization, HTTP clients, and schemas", "sort_order": 2},
    {"id": str(uuid.uuid4()), "name": "Database Queries & Migrations", "description": "SQL queries, schema migrations, and indexing patterns", "sort_order": 3},
    {"id": str(uuid.uuid4()), "name": "DevOps & Tooling", "description": "Shell scripts, CI/CD templates, and build configuration", "sort_order": 4}
]

def generate_dataset(count: int) -> dict:
    random.seed(42 + count)
    snippets = []
    all_tags = set(TAG_POOL)
    base_time = datetime.datetime(2026, 1, 1, 10, 0, 0, tzinfo=datetime.timezone.utc)

    for i in range(1, count + 1):
        lang, ext, body_template = random.choice(LANGUAGES)
        snip_type = random.choice(TYPES)
        col = random.choice(COLLECTIONS)
        tags = random.sample(TAG_POOL, k=random.randint(1, 4))
        for t in tags:
            all_tags.add(t)

        time_delta = datetime.timedelta(minutes=i * 15)
        created_at = (base_time + time_delta).isoformat(timespec="milliseconds")
        updated_at = (base_time + time_delta + datetime.timedelta(minutes=5)).isoformat(timespec="milliseconds")

        title = f"Snippet #{i:05d}: {lang} {snip_type.title()} Example"
        body = f"// {title}\n// Language: {lang}\n\n{body_template}\n\n// Iteration key: {i}"
        desc = f"Deterministic technical snippet fixture #{i} illustrating {snip_type} design for {lang}."

        snippet = {
            "id": str(uuid.uuid4()),
            "title": title,
            "body": body,
            "description": desc,
            "language": lang,
            "snippet_type": snip_type,
            "collection_id": col["id"],
            "favorite": (i % 7 == 0),
            "pinned": (i % 23 == 0),
            "created_at": created_at,
            "updated_at": updated_at,
            "source_url": f"https://example.com/docs/{lang.lower()}/item-{i}" if (i % 5 == 0) else "",
            "tags": tags
        }
        snippets.append(snippet)

    templates = [
        {
            "id": str(uuid.uuid4()),
            "name": "Standard C++ Class Template",
            "description": "Boilerplate for a C++ class with Rule of Five",
            "body": "#pragma once\n\nclass {{ClassName}} {\npublic:\n    {{ClassName}}() = default;\n    ~{{ClassName}}() = default;\n};",
            "language": "C++"
        },
        {
            "id": str(uuid.uuid4()),
            "name": "Python Async Function Template",
            "description": "Async function with error handling",
            "body": "async def {{function_name}}(*args, **kwargs):\n    try:\n        # implementation\n        pass\n    except Exception as exc:\n        raise",
            "language": "Python"
        }
    ]

    return {
        "format": "codeshelf-backup",
        "schema_version": 1,
        "exported_at": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="milliseconds"),
        "application_version": "0.1.0",
        "collections": COLLECTIONS,
        "tags": sorted(list(all_tags)),
        "templates": templates,
        "snippets": snippets
    }

def main():
    fixtures_dir = Path(__file__).resolve().parent.parent / "fixtures"
    fixtures_dir.mkdir(parents=True, exist_ok=True)

    for count in [100, 1000, 10000]:
        data = generate_dataset(count)
        out_file = fixtures_dir / f"snippets_{count}.json"
        print(f"Generating {out_file} ({count} snippets)...")
        with open(out_file, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)
        print(f"  -> Generated: {out_file.name} ({out_file.stat().st_size / 1024:.1f} KB)")

if __name__ == "__main__":
    main()
