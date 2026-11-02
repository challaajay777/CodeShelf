#pragma once

#include <QString>
#include <QVector>
#include <optional>
#include "../core/SmartView.h"
#include "../database/Database.h"

namespace CodeShelf {

class SmartViewRepository {
public:
    explicit SmartViewRepository(Database& db = Database::instance());

    bool create(SmartView& sv, QString& errorOut);
    bool update(const SmartView& sv, QString& errorOut);
    bool deleteSmartView(const QString& id, QString& errorOut);

    std::optional<SmartView> getById(const QString& id);
    QVector<SmartView> getAll();

private:
    Database& m_db;
};

} // namespace CodeShelf
