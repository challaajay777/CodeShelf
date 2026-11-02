#include "Tag.h"

namespace CodeShelf {

Tag::Tag()
    : m_id(Validation::generateUuid())
    , m_createdAt(QDateTime::currentDateTimeUtc())
{
}

Tag::Tag(const QString& displayName)
    : m_id(Validation::generateUuid())
    , m_normalizedName(Validation::normalizeTag(displayName))
    , m_displayName(displayName.trimmed())
    , m_createdAt(QDateTime::currentDateTimeUtc())
{
}

void Tag::setDisplayName(const QString& name) {
    m_displayName = name.trimmed();
    m_normalizedName = Validation::normalizeTag(m_displayName);
}

ValidationResult Tag::validate() const {
    return Validation::validateTagName(m_displayName);
}

} // namespace CodeShelf
