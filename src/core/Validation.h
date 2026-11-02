#pragma once

#include <QString>
#include <QStringList>
#include <QUuid>
#include <QUrl>

namespace CodeShelf {

struct ValidationResult {
    bool valid{true};
    QString errorMessage;

    static ValidationResult success() { return {true, QString()}; }
    static ValidationResult failure(const QString& msg) { return {false, msg}; }
};

class Validation {
public:
    static constexpr int MaxTitleLength = 255;
    static constexpr int MaxDescriptionLength = 4096;
    static constexpr int MaxBodyLength = 10 * 1024 * 1024; // 10 MB
    static constexpr int MaxTagNameLength = 64;
    static constexpr int MaxCollectionNameLength = 128;

    static ValidationResult validateSnippetTitle(const QString& title);
    static ValidationResult validateSnippetBody(const QString& body);
    static ValidationResult validateSourceUrl(const QString& url);
    static ValidationResult validateTagName(const QString& tag);
    static ValidationResult validateCollectionName(const QString& name);
    static ValidationResult validateUuid(const QString& uuid);

    static QString normalizeTag(const QString& rawTag);
    static QString sanitizeForDisplay(const QString& input);
    static QString generateUuid();
};

} // namespace CodeShelf
