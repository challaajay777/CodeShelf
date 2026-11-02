#pragma once

#include <QString>
#include <QVector>
#include <optional>
#include "../core/Collection.h"
#include "../database/Database.h"

namespace CodeShelf {

class CollectionRepository {
public:
    explicit CollectionRepository(Database& db = Database::instance());

    bool create(Collection& collection, QString& errorOut);
    bool update(const Collection& collection, QString& errorOut);
    bool deleteCollection(const QString& id, bool deleteSnippets, QString& errorOut);

    std::optional<Collection> getById(const QString& id);
    QVector<Collection> getAll();
    int getSnippetCount(const QString& collectionId);

private:
    Database& m_db;
};

} // namespace CodeShelf
