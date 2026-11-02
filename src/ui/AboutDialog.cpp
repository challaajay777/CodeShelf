#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace CodeShelf {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("About CodeShelf"));
    resize(460, 300);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto titleLabel = new QLabel(QStringLiteral("<h2>CodeShelf v0.1.0</h2>"), this);
    layout->addWidget(titleLabel);

    auto descLabel = new QLabel(
        QStringLiteral(
            "<b>A dependable, offline-first native desktop snippet manager.</b><br><br>"
            "Built with C++20, Qt 6 Widgets, SQLite 3, and CMake.<br><br>"
            "<b>Key Principles:</b><br>"
            "• Local-First: Entirely functional offline; no telemetry or accounts.<br>"
            "• Inert Text: Snippets are plain text and never compiled or executed.<br>"
            "• Data Integrity: ACID transactions, foreign keys, and WAL-safe backups.<br>"
            "• Minimal UI: Clean native controls with zero AI/neon clutter.<br><br>"
            "Released under the MIT License."
        ), this);
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    auto closeBtn = new QPushButton(QStringLiteral("Close"), this);
    closeBtn->setDefault(true);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

} // namespace CodeShelf
