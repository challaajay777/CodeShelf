#pragma once

#include <QObject>
#include <QTest>
#include "core/Validation.h"

namespace CodeShelf {

class TestValidation : public QObject {
    Q_OBJECT

private slots:
    void testTitleValidation();
    void testSourceUrlValidation();
    void testTagValidationAndNormalization();
    void testCollectionNameValidation();
};

} // namespace CodeShelf
