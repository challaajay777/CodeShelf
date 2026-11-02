#pragma once

#include <QString>
#include <QVector>
#include <optional>
#include "../core/Template.h"
#include "../database/Database.h"

namespace CodeShelf {

class TemplateRepository {
public:
    explicit TemplateRepository(Database& db = Database::instance());

    bool create(Template& tmpl, QString& errorOut);
    bool update(const Template& tmpl, QString& errorOut);
    bool deleteTemplate(const QString& id, QString& errorOut);

    std::optional<Template> getById(const QString& id);
    QVector<Template> getAll();

private:
    Database& m_db;
};

} // namespace CodeShelf
