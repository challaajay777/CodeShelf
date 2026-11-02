#include "PlatformUtils.h"
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>
#include "../core/Validation.h"

namespace CodeShelf {

bool PlatformUtils::openSafeUrl(const QString& rawUrl, QString& errorOut) {
    auto val = Validation::validateSourceUrl(rawUrl);
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QUrl qurl(rawUrl.trimmed());
    const QString scheme = qurl.scheme().toLower();
    if (scheme != QStringLiteral("http") && scheme != QStringLiteral("https")) {
        errorOut = QStringLiteral("Disallowed scheme for security. Only http and https URLs may be opened.");
        return false;
    }

    if (!QDesktopServices::openUrl(qurl)) {
        errorOut = QStringLiteral("Failed to open URL in system browser.");
        return false;
    }

    return true;
}

QString PlatformUtils::defaultDataDirectory() {
    const QString loc = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(loc);
    return loc;
}

QString PlatformUtils::defaultBackupDirectory() {
    const QString loc = defaultDataDirectory() + QStringLiteral("/backups");
    QDir().mkpath(loc);
    return loc;
}

} // namespace CodeShelf
