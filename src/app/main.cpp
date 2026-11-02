#include <QApplication>
#include <QMessageBox>
#include <iostream>
#include "../database/Database.h"
#include "../ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("CodeShelf"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationName(QStringLiteral("CodeShelf"));
    app.setOrganizationDomain(QStringLiteral("codeshelf.local"));

    // Open default database
    auto& db = CodeShelf::Database::instance();
    if (!db.open()) {
        QMessageBox::critical(nullptr, QStringLiteral("Database Initialization Error"),
                              QStringLiteral("Failed to open local CodeShelf database:\n%1").arg(db.lastError()));
        return 1;
    }

    CodeShelf::MainWindow mainWindow;
    mainWindow.show();

    int exitCode = app.exec();

    // Deterministic database shutdown
    db.close();

    return exitCode;
}
