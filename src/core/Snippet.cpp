#include "Snippet.h"

namespace CodeShelf {

Snippet::Snippet()
    : m_id(Validation::generateUuid())
    , m_createdAt(QDateTime::currentDateTimeUtc())
    , m_updatedAt(m_createdAt)
{
    refreshContentHash();
}

Snippet::Snippet(const QString& title, const QString& body, const QString& language)
    : m_id(Validation::generateUuid())
    , m_title(title)
    , m_body(body)
    , m_language(normalizeLanguage(language))
    , m_createdAt(QDateTime::currentDateTimeUtc())
    , m_updatedAt(m_createdAt)
{
    refreshContentHash();
}

void Snippet::setBody(const QString& body) {
    m_body = body;
    refreshContentHash();
}

void Snippet::refreshContentHash() {
    m_contentHash = ContentHash::computeExactHash(m_body);
}

void Snippet::setTags(const QStringList& tags) {
    QStringList normalized;
    for (const QString& t : tags) {
        const QString norm = t.trimmed();
        if (!norm.isEmpty() && !normalized.contains(norm, Qt::CaseInsensitive)) {
            normalized.append(norm);
        }
    }
    m_tags = normalized;
}

void Snippet::addTag(const QString& tag) {
    const QString norm = tag.trimmed();
    if (!norm.isEmpty() && !m_tags.contains(norm, Qt::CaseInsensitive)) {
        m_tags.append(norm);
    }
}

void Snippet::removeTag(const QString& tag) {
    for (int i = 0; i < m_tags.size(); ++i) {
        if (m_tags[i].compare(tag.trimmed(), Qt::CaseInsensitive) == 0) {
            m_tags.removeAt(i);
            break;
        }
    }
}

ValidationResult Snippet::validate() const {
    auto titleRes = Validation::validateSnippetTitle(m_title);
    if (!titleRes.valid) return titleRes;

    auto bodyRes = Validation::validateSnippetBody(m_body);
    if (!bodyRes.valid) return bodyRes;

    auto urlRes = Validation::validateSourceUrl(m_sourceUrl);
    if (!urlRes.valid) return urlRes;

    for (const QString& tag : m_tags) {
        auto tagRes = Validation::validateTagName(tag);
        if (!tagRes.valid) return tagRes;
    }

    return ValidationResult::success();
}

} // namespace CodeShelf
