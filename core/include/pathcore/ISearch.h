#pragma once

#include <cstddef>

#include "pathcore/Grid.h"
#include "pathcore/SearchConfig.h"
#include "pathcore/SearchSnapshot.h"
#include "pathcore/SearchStatus.h"

namespace pathcore {

class ISearch {
public:
    virtual ~ISearch() = default;

    virtual bool reset(const Grid& grid, CellPos start, CellPos goal, const SearchConfig& config) = 0;
    virtual SearchStatus step(std::size_t iterations = 1) = 0;
    virtual SearchStatus status() const = 0;
    virtual const SearchSnapshot& snapshot() const = 0;
};

} // namespace pathcore
