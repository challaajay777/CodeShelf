#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDateTime>
#include "Validation.h"
#include "Enums.h"

namespace CodeShelf {

class Template {
public:
    Template();
    explicit Template(const QString& name, const QString& body, const QString& language = QStringLiteral("Plain Text"), const QString& description = QString());

    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QString description() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; }

    QString body() const { return m_body; }
    void setBody(const QString& body) { m_body = body; }

    QString language() const { return m_language; }
    void setLanguage(const QString& lang) { m_language = normalizeLanguage(lang); }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    QDateTime updatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

    // Extracts all placeholder variable names found in {{varName}}
    QStringList extractPlaceholders() const;

    // Evaluates substitutions without any expression evaluation or execution
    QString applySubstitutions(const QMap<QString, QString>& values) const;

    ValidationResult validate() const;

private:
    QString m_id;
    QString m_name;
    QString m_description;
    QString m_body;
    QString m_language{QStringLiteral("Plain Text")};
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};

} // namespace CodeShelf
