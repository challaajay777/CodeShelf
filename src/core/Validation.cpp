#include "Validation.h"
#include <QRegularExpression>

namespace CodeShelf {

ValidationResult Validation::validateSnippetTitle(const QString& title) {
    const QString trimmed = title.trimmed();
    if (trimmed.isEmpty()) {
        return ValidationResult::failure(QStringLiteral("Snippet title cannot be empty."));
    }
    if (title.length() > MaxTitleLength) {
        return ValidationResult::failure(QStringLiteral("Snippet title exceeds maximum length of %1 characters.").arg(MaxTitleLength));
    }
    return ValidationResult::success();
}

ValidationResult Validation::validateSnippetBody(const QString& body) {
    if (body.size() > MaxBodyLength) {
        return ValidationResult::failure(QStringLiteral("Snippet body exceeds maximum allowed size of 10 MB."));
    }
    return ValidationResult::success();
}

ValidationResult Validation::validateSourceUrl(const QString& url) {
    const QString trimmed = url.trimmed();
    if (trimmed.isEmpty()) {
        return ValidationResult::success(); // Optional field
    }

    QUrl qurl(trimmed);
    if (!qurl.isValid()) {
        return ValidationResult::failure(QStringLiteral("Invalid URL format."));
    }

    const QString scheme = qurl.scheme().toLower();
    if (scheme != QStringLiteral("http") && scheme != QStringLiteral("https")) {
        return ValidationResult::failure(QStringLiteral("Source URL must use http or https scheme."));
    }

    return ValidationResult::success();
}

ValidationResult Validation::validateTagName(const QString& tag) {
    const QString trimmed = tag.trimmed();
    if (trimmed.isEmpty()) {
        return ValidationResult::failure(QStringLiteral("Tag name cannot be empty."));
    }
    if (trimmed.length() > MaxTagNameLength) {
        return ValidationResult::failure(QStringLiteral("Tag name exceeds maximum length of %1 characters.").arg(MaxTagNameLength));
    }
    if (trimmed.contains(QLatin1Char(',')) || trimmed.contains(QLatin1Char(';')) || trimmed.contains(QLatin1Char('\n'))) {
        return ValidationResult::failure(QStringLiteral("Tag name cannot contain commas, semicolons, or newlines."));
    }
    return ValidationResult::success();
}

ValidationResult Validation::validateCollectionName(const QString& name) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return ValidationResult::failure(QStringLiteral("Collection name cannot be empty."));
    }
    if (trimmed.length() > MaxCollectionNameLength) {
        return ValidationResult::failure(QStringLiteral("Collection name exceeds maximum length of %1 characters.").arg(MaxCollectionNameLength));
    }
    return ValidationResult::success();
}

ValidationResult Validation::validateUuid(const QString& uuid) {
    QUuid quuid(uuid);
    if (quuid.isNull() && uuid != QStringLiteral("{00000000-0000-0000-0000-000000000000}")) {
        return ValidationResult::failure(QStringLiteral("Invalid UUID string."));
    }
    return ValidationResult::success();
}

QString Validation::normalizeTag(const QString& rawTag) {
    return rawTag.trimmed().toLower();
}

QString Validation::sanitizeForDisplay(const QString& input) {
    return input.trimmed();
}

QString Validation::generateUuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace CodeShelf
