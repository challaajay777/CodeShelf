#include "TemplateService.h"

namespace CodeShelf {

TemplateService::TemplateService(TemplateRepository& repo)
    : m_repo(repo)
{
}

bool TemplateService::saveTemplate(Template& tmpl, QString& errorOut) {
    auto existing = m_repo.getById(tmpl.id());
    if (existing.has_value()) {
        return m_repo.update(tmpl, errorOut);
    }
    return m_repo.create(tmpl, errorOut);
}

bool TemplateService::deleteTemplate(const QString& id, QString& errorOut) {
    return m_repo.deleteTemplate(id, errorOut);
}

QVector<Template> TemplateService::getAllTemplates() {
    return m_repo.getAll();
}

} // namespace CodeShelf
