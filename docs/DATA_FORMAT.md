# CodeShelf Data Exchange Formats

This document defines the official data exchange formats supported by CodeShelf: the versioned JSON backup format (`codeshelf-backup`) and the safe Markdown export format.

---

## 1. JSON Backup Specification (`codeshelf-backup`)

The primary format for complete database backups, cross-machine transfers, and automated imports/exports is a versioned JSON payload.

### 1.1. Top-Level Structure

```json
{
  "format": "codeshelf-backup",
  "schema_version": 1,
  "exported_at": "2026-11-02T10:30:00.000Z",
  "application_version": "0.1.0",
  "collections": [ ... ],
  "tags": [ ... ],
  "templates": [ ... ],
  "snippets": [ ... ]
}
```

### 1.2. Field Definitions

#### Top-Level Fields
| Field | Type | Required | Description |
| :--- | :--- | :--- | :--- |
| `format` | String | Yes | Fixed string `"codeshelf-backup"`. Identifies file format. |
| `schema_version` | Integer | Yes | Schema version integer. Currently `1`. |
| `exported_at` | String | Yes | ISO 8601 UTC timestamp of export (`YYYY-MM-DDTHH:MM:SS.mmmZ`). |
| `application_version` | String | Yes | Version of CodeShelf that generated the backup. |
| `collections` | Array | Yes | List of collection objects. |
| `tags` | Array | Yes | List of unique tag display names. |
| `templates` | Array | No | Optional list of template objects. |
| `snippets` | Array | Yes | List of snippet objects. |

#### Snippet Object
```json
{
  "id": "e2a1b3c4-d5e6-7890-abcd-ef0123456789",
  "title": "Quick Sort Implementation",
  "body": "template<typename T>\nvoid quickSort(std::vector<T>& arr) {\n    // ...\n}",
  "description": "Standard in-place QuickSort algorithm in modern C++",
  "language": "C++",
  "snippet_type": "code",
  "collection_id": "c1b2a3d4-e5f6-7890-abcd-ef0123456789",
  "favorite": true,
  "pinned": false,
  "created_at": "2026-01-15T08:30:00.000Z",
  "updated_at": "2026-03-20T14:45:00.000Z",
  "source_url": "https://example.com/algorithms/quicksort",
  "tags": ["algorithms", "sorting", "modern-cpp"]
}
```

| Field | Type | Required | Constraints |
| :--- | :--- | :--- | :--- |
| `id` | String | Yes | Valid UUID v4 string. |
| `title` | String | Yes | 1 to 255 characters. |
| `body` | String | Yes | Full plain text code/note content. Preserves whitespace and line endings. |
| `description` | String | No | Optional markdown or plain text notes. Max 2048 characters. |
| `language` | String | Yes | Standard language identifier (e.g., `C++`, `Python`, `Rust`, `SQL`, `Plain Text`). |
| `snippet_type` | String | Yes | One of: `code`, `note`, `command`, `config`, `sql`, `pseudocode`. |
| `collection_id` | String | No | Null or matches an ID in `collections`. |
| `favorite` | Boolean | Yes | `true` if starred/favorited. |
| `pinned` | Boolean | Yes | `true` if pinned to the top of listings. |
| `created_at` | String | Yes | ISO 8601 UTC timestamp. |
| `updated_at` | String | Yes | ISO 8601 UTC timestamp. |
| `source_url` | String | No | Null or valid URL strictly with `http://` or `https://` protocol. |
| `tags` | Array of Strings| Yes | List of tag strings. Max 50 characters each. |

#### Collection Object
```json
{
  "id": "c1b2a3d4-e5f6-7890-abcd-ef0123456789",
  "name": "Algorithms & Data Structures",
  "description": "Fundamental computer science routines",
  "sort_order": 0
}
```

#### Template Object
```json
{
  "id": "t1b2a3d4-e5f6-7890-abcd-ef0123456789",
  "name": "C++ Header Guard",
  "description": "Boilerplate for new header files",
  "body": "#ifndef {{NAME}}_H\n#define {{NAME}}_H\n\n// TODO\n\n#endif // {{NAME}}_H",
  "language": "C++"
}
```

---

## 2. Markdown Export Specification

CodeShelf provides human-readable Markdown export for individual snippets or multi-snippet documents.

### 2.1. Code-Fence Escaping Protection
To prevent Markdown code-fence injection (where snippet code itself contains ```` ``` ```` backticks), CodeShelf uses dynamic outer code fences:
- If snippet content contains ```` ``` ````, outer fence uses ```` ```` ```` (quadruple backticks).
- If snippet content contains ```` ```` ````, outer fence uses ```` ````` ```` (quintuple backticks).

### 2.2. Snippet Markdown Layout
```markdown
# Snippet Title

- **Language:** C++
- **Type:** code
- **Collection:** Algorithms
- **Tags:** `algorithms`, `sorting`
- **Created:** 2026-01-15 08:30:00 UTC
- **Source:** [https://example.com/algorithms](https://example.com/algorithms)

## Description
Standard in-place QuickSort algorithm in modern C++.

## Content
````cpp
template<typename T>
void quickSort(std::vector<T>& arr) {
    // ...
}
````
```

---

## 3. Import Conflict Resolution Strategies

When importing a backup or snippet bundle that contains identifiers or content matching existing entries, CodeShelf provides an interactive preview showing:
- **New Entries**: IDs or content not currently in the database.
- **Duplicate Entries**: Same content hash already present.
- **Conflicting Entries**: Same ID but differing content or updated timestamp.

### Supported Strategies:
1. **Skip Conflicting**: Keeps existing database record unchanged; ignores conflicting incoming record.
2. **Keep Both**: Generates a new random UUID v4 for incoming record, appending ` (Imported Copy)` to the title to avoid collision.
3. **Update / Overwrite**: Updates the existing snippet with the imported record and generates an automatic historical revision snapshot of the overwritten version.
