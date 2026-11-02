#include "Migrations.h"
#include "SqliteHelper.h"

namespace CodeShelf {

int Migrations::getCurrentVersion(sqlite3* db) {
    if (!db) return -1;
    sqlite3_stmt* stmt = nullptr;
    int version = 0;
    if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            version = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return version;
}

bool Migrations::setVersion(sqlite3* db, int version) {
    if (!db) return false;
    const QString sql = QStringLiteral("PRAGMA user_version = %1;").arg(version);
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.toUtf8().constData(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Migrations::runMigrations(sqlite3* db, bool fts5Available, QString& errorOut) {
    int current = getCurrentVersion(db);
    if (current < 0) {
        errorOut = QStringLiteral("Failed to read schema version.");
        return false;
    }

    if (current == CurrentSchemaVersion) {
        return true;
    }

    if (current > CurrentSchemaVersion) {
        errorOut = QStringLiteral("Database was created with a newer schema version (%1). Please upgrade CodeShelf.").arg(current);
        return false;
    }

    // Step-by-step migrations
    if (current < 1) {
        if (!migrateToV1(db, fts5Available, errorOut)) {
            return false;
        }
    }

    return true;
}

bool Migrations::migrateToV1(sqlite3* db, bool fts5Available, QString& errorOut) {
    SqliteTransaction tx(db);

    const char* v1SchemaSql = R"(
        CREATE TABLE IF NOT EXISTS schema_metadata (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS collections (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            sort_order INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS tags (
            id TEXT PRIMARY KEY,
            normalized_name TEXT NOT NULL UNIQUE,
            display_name TEXT NOT NULL,
            created_at TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS snippets (
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

        CREATE TABLE IF NOT EXISTS snippet_tags (
            snippet_id TEXT NOT NULL REFERENCES snippets(id) ON DELETE CASCADE,
            tag_id TEXT NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
            PRIMARY KEY (snippet_id, tag_id)
        );

        CREATE TABLE IF NOT EXISTS snippet_revisions (
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

        CREATE TABLE IF NOT EXISTS templates (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            body TEXT NOT NULL,
            language TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS smart_views (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            query_string TEXT NOT NULL,
            sort_order INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL
        );

        CREATE INDEX IF NOT EXISTS idx_snippets_collection ON snippets(collection_id);
        CREATE INDEX IF NOT EXISTS idx_snippets_language ON snippets(language);
        CREATE INDEX IF NOT EXISTS idx_snippets_favorite ON snippets(favorite);
        CREATE INDEX IF NOT EXISTS idx_snippets_pinned ON snippets(pinned);
        CREATE INDEX IF NOT EXISTS idx_snippets_deleted_at ON snippets(deleted_at);
        CREATE INDEX IF NOT EXISTS idx_snippets_updated_at ON snippets(updated_at);
        CREATE INDEX IF NOT EXISTS idx_snippets_content_hash ON snippets(content_hash);
        CREATE INDEX IF NOT EXISTS idx_snippet_tags_tag ON snippet_tags(tag_id);
        CREATE INDEX IF NOT EXISTS idx_revisions_snippet ON snippet_revisions(snippet_id, revision_number);
    )";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, v1SchemaSql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        errorOut = errMsg ? QString::fromUtf8(errMsg) : QStringLiteral("Failed executing V1 schema SQL.");
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }

    // Set schema metadata
    const char* metaSql = "INSERT OR REPLACE INTO schema_metadata (key, value) VALUES ('version', '1'), ('app', 'CodeShelf');";
    if (sqlite3_exec(db, metaSql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        errorOut = errMsg ? QString::fromUtf8(errMsg) : QStringLiteral("Failed setting schema metadata.");
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }

    if (fts5Available) {
        const char* ftsSql = R"(
            CREATE VIRTUAL TABLE IF NOT EXISTS snippets_fts USING fts5(
                snippet_id UNINDEXED,
                title,
                description,
                body,
                tags,
                tokenize = 'porter unicode61'
            );
        )";
        if (sqlite3_exec(db, ftsSql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            // Non-fatal warning if FTS table creation fails
            if (errMsg) sqlite3_free(errMsg);
        }
    }

    if (!setVersion(db, 1)) {
        errorOut = QStringLiteral("Failed to update user_version to 1.");
        return false;
    }

    if (!tx.commit()) {
        errorOut = QStringLiteral("Failed to commit V1 migration transaction.");
        return false;
    }

    return true;
}

} // namespace CodeShelf
