#include "Template.h"
#include <QRegularExpression>

namespace CodeShelf {

Template::Template()
    : m_id(Validation::generateUuid())
    , m_createdAt(QDateTime::currentDateTimeUtc())
    , m_updatedAt(m_createdAt)
{
}

Template::Template(const QString& name, const QString& body, const QString& language, const QString& description)
    : m_id(Validation::generateUuid())
    , m_name(name)
    , m_description(description)
    , m_body(body)
    , m_language(normalizeLanguage(language))
    , m_createdAt(QDateTime::currentDateTimeUtc())
    , m_updatedAt(m_createdAt)
{
}

QStringList Template::extractPlaceholders() const {
    QStringList placeholders;
    // Match {{placeholder_name}}
    static const QRegularExpression regex(QStringLiteral(R"(\{\{([a-zA-Z0-9_-]+)\}\})"));
    auto it = regex.globalMatch(m_body);
    while (it.hasNext()) {
        auto match = it.next();
        const QString varName = match.captured(1);
        if (!placeholders.contains(varName)) {
            placeholders.append(varName);
        }
    }
    return placeholders;
}

QString Template::applySubstitutions(const QMap<QString, QString>& values) const {
    QString result = m_body;
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        const QString tag = QStringLiteral("{{%1}}").arg(it.key());
        result.replace(tag, it.value());
    }
    return result;
}

ValidationResult Template::validate() const {
    if (m_name.trimmed().isEmpty()) {
        return ValidationResult::failure(QStringLiteral("Template name cannot be empty."));
    }
    if (m_name.length() > 128) {
        return ValidationResult::failure(QStringLiteral("Template name exceeds 128 characters."));
    }
    return ValidationResult::success();
}

} // namespace CodeShelf
