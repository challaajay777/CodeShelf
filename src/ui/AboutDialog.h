#pragma once

#include <QDialog>

namespace CodeShelf {

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

} // namespace CodeShelf
