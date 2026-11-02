#pragma once

#include "SearchQuery.h"

namespace CodeShelf {

class SearchParser {
public:
    static SearchQuery parse(const QString& input);
};

} // namespace CodeShelf
