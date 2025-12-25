#pragma once

#include <cassert>

#include "pathcore/Grid.h"
#include "pathcore/SearchConfig.h"
#include "pathcore/SearchSnapshot.h"
#include "pathcore/SearchStatus.h"
#include "pathcore/Types.h"

namespace pathcore {

class SearchBase {
public:
    bool commonReset(const Grid& grid, CellPos start, CellPos goal, const SearchConfig& config) {
        grid_ = nullptr;
        start_ = {};
        goal_ = {};
        status_ = SearchStatus::NotStarted;

        if (grid.width() <= 0 || grid.height() <= 0) {
            snapshot_.resize(0, 0);
            return false;
        }
        if (!grid.inBounds(start) || !grid.inBounds(goal)) {
            snapshot_.resize(0, 0);
            return false;
        }
        if (grid.isBlocked(start) || grid.isBlocked(goal)) {
            snapshot_.resize(0, 0);
            return false;
        }

        grid_ = &grid;
        start_ = start;
        goal_ = goal;
        config_ = config;
        snapshot_.resize(grid.width(), grid.height());
        snapshot_.clear();
        status_ = SearchStatus::Running;
        return true;
    }

    SearchStatus status() const {
        return status_;
    }

    const SearchSnapshot& snapshot() const {
        return snapshot_;
    }

    const Grid& grid() const {
        assert(grid_ != nullptr);
        return *grid_;
    }

    CellPos start() const {
        return start_;
    }

    CellPos goal() const {
        return goal_;
    }

protected:
    const Grid* grid_{nullptr};
    CellPos start_{};
    CellPos goal_{};
    SearchConfig config_{};
    SearchStatus status_{SearchStatus::NotStarted};
    SearchSnapshot snapshot_{};
};

} // namespace pathcore
