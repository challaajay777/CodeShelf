#pragma once

#include <QString>
#include <QVector>
#include "../core/Template.h"
#include "../repositories/TemplateRepository.h"

namespace CodeShelf {

class TemplateService {
public:
    explicit TemplateService(TemplateRepository& repo);

    bool saveTemplate(Template& tmpl, QString& errorOut);
    bool deleteTemplate(const QString& id, QString& errorOut);
    QVector<Template> getAllTemplates();

private:
    TemplateRepository& m_repo;
};

} // namespace CodeShelf
