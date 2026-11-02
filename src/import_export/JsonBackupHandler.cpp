#include "JsonBackupHandler.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include "../core/ContentHash.h"

namespace CodeShelf {

JsonBackupHandler::JsonBackupHandler(SnippetRepository& snippetRepo, CollectionRepository& colRepo,
                                     TagRepository& tagRepo, TemplateRepository& tmplRepo)
    : m_snippetRepo(snippetRepo)
    , m_colRepo(colRepo)
    , m_tagRepo(tagRepo)
    , m_tmplRepo(tmplRepo)
{
}

bool JsonBackupHandler::exportBackup(const QString& targetFilePath, const QVector<QString>& snippetIdsToExport,
                                     bool /*includeRevisions*/, bool includeTemplates, QString& errorOut) {
    QJsonObject root;
    root[QStringLiteral("format")] = QStringLiteral("codeshelf-backup");
    root[QStringLiteral("schema_version")] = 1;
    root[QStringLiteral("exported_at")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    root[QStringLiteral("application_version")] = QStringLiteral("0.1.0");

    // Collections
    QJsonArray colArray;
    const auto collections = m_colRepo.getAll();
    for (const auto& col : collections) {
        QJsonObject obj;
        obj[QStringLiteral("id")] = col.id();
        obj[QStringLiteral("name")] = col.name();
        obj[QStringLiteral("description")] = col.description();
        obj[QStringLiteral("sort_order")] = col.sortOrder();
        colArray.append(obj);
    }
    root[QStringLiteral("collections")] = colArray;

    // Snippets
    QVector<Snippet> snippetsToSave;
    if (snippetIdsToExport.isEmpty()) {
        snippetsToSave = m_snippetRepo.getAllActive();
    } else {
        for (const QString& id : snippetIdsToExport) {
            auto s = m_snippetRepo.getById(id);
            if (s.has_value()) {
                snippetsToSave.append(*s);
            }
        }
    }

    QJsonArray snipArray;
    QSet<QString> allTags;
    for (const auto& snip : snippetsToSave) {
        QJsonObject obj;
        obj[QStringLiteral("id")] = snip.id();
        obj[QStringLiteral("title")] = snip.title();
        obj[QStringLiteral("body")] = snip.body();
        obj[QStringLiteral("description")] = snip.description();
        obj[QStringLiteral("language")] = snip.language();
        obj[QStringLiteral("snippet_type")] = snippetTypeToString(snip.snippetType());
        obj[QStringLiteral("collection_id")] = snip.collectionId();
        obj[QStringLiteral("favorite")] = snip.isFavorite();
        obj[QStringLiteral("pinned")] = snip.isPinned();
        obj[QStringLiteral("created_at")] = snip.createdAt().toString(Qt::ISODateWithMs);
        obj[QStringLiteral("updated_at")] = snip.updatedAt().toString(Qt::ISODateWithMs);
        obj[QStringLiteral("source_url")] = snip.sourceUrl();

        QJsonArray tagArr;
        for (const QString& t : snip.tags()) {
            tagArr.append(t);
            allTags.insert(t);
        }
        obj[QStringLiteral("tags")] = tagArr;

        snipArray.append(obj);
    }
    root[QStringLiteral("snippets")] = snipArray;

    // Tags list
    QJsonArray tagArray;
    for (const QString& t : allTags) {
        tagArray.append(t);
    }
    root[QStringLiteral("tags")] = tagArray;

    // Templates
    if (includeTemplates) {
        QJsonArray tmplArray;
        const auto tmpls = m_tmplRepo.getAll();
        for (const auto& tmpl : tmpls) {
            QJsonObject obj;
            obj[QStringLiteral("id")] = tmpl.id();
            obj[QStringLiteral("name")] = tmpl.name();
            obj[QStringLiteral("description")] = tmpl.description();
            obj[QStringLiteral("body")] = tmpl.body();
            obj[QStringLiteral("language")] = tmpl.language();
            tmplArray.append(obj);
        }
        root[QStringLiteral("templates")] = tmplArray;
    }

    // Atomic write
    const QString tempPath = targetFilePath + QStringLiteral(".tmp");
    QFile file(tempPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorOut = QStringLiteral("Cannot open file for writing: %1").arg(targetFilePath);
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    if (QFile::exists(targetFilePath)) {
        QFile::remove(targetFilePath);
    }
    if (!QFile::rename(tempPath, targetFilePath)) {
        errorOut = QStringLiteral("Failed to atomically finalize export file.");
        return false;
    }

    return true;
}

bool JsonBackupHandler::parseAndPreview(const QString& sourceFilePath, ImportPreviewSummary& previewOut, QString& errorOut) {
    QFile file(sourceFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorOut = QStringLiteral("Cannot open backup file: %1").arg(sourceFilePath);
        return false;
    }

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    file.close();

    if (parseErr.error != QJsonParseError::NoError) {
        errorOut = QStringLiteral("JSON syntax error: %1").arg(parseErr.errorString());
        return false;
    }

    if (!doc.isObject()) {
        errorOut = QStringLiteral("Invalid backup file: root must be a JSON object.");
        return false;
    }

    QJsonObject root = doc.object();
    if (root.value(QStringLiteral("format")).toString() != QStringLiteral("codeshelf-backup")) {
        errorOut = QStringLiteral("Invalid backup format signature.");
        return false;
    }

    int schemaVer = root.value(QStringLiteral("schema_version")).toInt();
    if (schemaVer <= 0 || schemaVer > 1) {
        errorOut = QStringLiteral("Unsupported backup schema version: %1").arg(schemaVer);
        return false;
    }

    previewOut = ImportPreviewSummary();

    // Parse collections
    if (root.contains(QStringLiteral("collections")) && root[QStringLiteral("collections")].isArray()) {
        QJsonArray colArr = root[QStringLiteral("collections")].toArray();
        for (const auto& val : colArr) {
            QJsonObject obj = val.toObject();
            Collection c;
            c.setId(obj.value(QStringLiteral("id")).toString());
            c.setName(obj.value(QStringLiteral("name")).toString());
            c.setDescription(obj.value(QStringLiteral("description")).toString());
            c.setSortOrder(obj.value(QStringLiteral("sort_order")).toInt());
            if (c.validate().valid) {
                previewOut.collections.append(c);
            }
        }
    }

    // Parse snippets and classify conflicts
    if (root.contains(QStringLiteral("snippets")) && root[QStringLiteral("snippets")].isArray()) {
        QJsonArray snipArr = root[QStringLiteral("snippets")].toArray();
        previewOut.totalInFile = snipArr.size();

        for (const auto& val : snipArr) {
            QJsonObject obj = val.toObject();
            ImportSnippetItem item;

            Snippet s;
            s.setId(obj.value(QStringLiteral("id")).toString());
            s.setTitle(obj.value(QStringLiteral("title")).toString());
            s.setBody(obj.value(QStringLiteral("body")).toString());
            s.setDescription(obj.value(QStringLiteral("description")).toString());
            s.setLanguage(obj.value(QStringLiteral("language")).toString());
            s.setSnippetType(stringToSnippetType(obj.value(QStringLiteral("snippet_type")).toString()));
            s.setCollectionId(obj.value(QStringLiteral("collection_id")).toString());
            s.setFavorite(obj.value(QStringLiteral("favorite")).toBool());
            s.setPinned(obj.value(QStringLiteral("pinned")).toBool());
            s.setSourceUrl(obj.value(QStringLiteral("source_url")).toString());

            QStringList tags;
            QJsonArray tagArr = obj.value(QStringLiteral("tags")).toArray();
            for (const auto& t : tagArr) {
                tags.append(t.toString());
            }
            s.setTags(tags);

            auto valRes = s.validate();
            if (!valRes.valid) {
                item.snippet = s;
                item.status = ImportItemStatus::Invalid;
                item.reason = valRes.errorMessage;
                previewOut.invalidCount++;
                previewOut.snippetItems.append(item);
                continue;
            }

            // Check duplicate by ID
            auto existingById = m_snippetRepo.getById(s.id());
            if (existingById.has_value()) {
                item.snippet = s;
                item.status = ImportItemStatus::DuplicateIdMatch;
                item.existingSnippetId = existingById->id();
                item.reason = QStringLiteral("Snippet with identical ID already exists: \"%1\"").arg(existingById->title());
                previewOut.duplicateIdCount++;
            } else {
                // Check duplicate by exact content
                auto dupsByHash = m_snippetRepo.findByContentHash(ContentHash::computeExactHash(s.body()));
                if (!dupsByHash.isEmpty()) {
                    item.snippet = s;
                    item.status = ImportItemStatus::DuplicateContentMatch;
                    item.existingSnippetId = dupsByHash.first().id();
                    item.reason = QStringLiteral("Snippet with identical code content exists: \"%1\"").arg(dupsByHash.first().title());
                    previewOut.duplicateContentCount++;
                } else {
                    item.snippet = s;
                    item.status = ImportItemStatus::NewItem;
                    previewOut.newCount++;
                }
            }

            previewOut.snippetItems.append(item);
        }
    }

    return true;
}

bool JsonBackupHandler::applyImport(ImportPreviewSummary& preview, ConflictResolution globalDefaultResolution, QString& errorOut) {
    // 1. Ensure collections exist
    for (Collection& col : preview.collections) {
        if (!m_colRepo.getById(col.id()).has_value()) {
            QString err;
            m_colRepo.create(col, err);
        }
    }

    // 2. Import snippets based on chosen resolution
    for (ImportSnippetItem& item : preview.snippetItems) {
        if (item.status == ImportItemStatus::Invalid) {
            continue; // Skip invalid records
        }

        ConflictResolution res = (item.resolution == ConflictResolution::Skip && globalDefaultResolution != ConflictResolution::Skip)
                                     ? globalDefaultResolution
                                     : item.resolution;

        if (item.status == ImportItemStatus::NewItem) {
            QString err;
            if (!m_snippetRepo.create(item.snippet, err)) {
                errorOut = QStringLiteral("Failed importing snippet '%1': %2").arg(item.snippet.title(), err);
                return false;
            }
        } else {
            // It is duplicate ID or duplicate content
            switch (res) {
                case ConflictResolution::Skip:
                    // Skip
                    break;
                case ConflictResolution::KeepBothNewId: {
                    Snippet copy = item.snippet;
                    copy.setId(Validation::generateUuid());
                    copy.setTitle(copy.title() + QStringLiteral(" (Imported)"));
                    QString err;
                    if (!m_snippetRepo.create(copy, err)) {
                        errorOut = err;
                        return false;
                    }
                    break;
                }
                case ConflictResolution::UpdateExisting: {
                    Snippet updateSnip = item.snippet;
                    if (!item.existingSnippetId.isEmpty()) {
                        updateSnip.setId(item.existingSnippetId);
                    }
                    QString err;
                    if (!m_snippetRepo.update(updateSnip, true, err, QStringLiteral("Updated from JSON import"))) {
                        errorOut = err;
                        return false;
                    }
                    break;
                }
            }
        }
    }

    return true;
}

} // namespace CodeShelf
