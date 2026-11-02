#pragma once

#include <QString>
#include <QUrl>

namespace CodeShelf {

class PlatformUtils {
public:
    // Safely opens HTTP or HTTPS URLs in the user's default web browser.
    // Explicitly rejects any file://, javascript:, data:, or executable scheme.
    static bool openSafeUrl(const QString& rawUrl, QString& errorOut);

    // Standard application data folder
    static QString defaultDataDirectory();

    // Default backup directory
    static QString defaultBackupDirectory();
};

} // namespace CodeShelf
