#pragma once

#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>

#include "pathcore/ISearch.h"
#include "pathcore/SearchBase.h"

namespace pathcore {

class Dijkstra final : public ISearch, public SearchBase {
public:
    bool reset(const Grid& grid, CellPos start, CellPos goal, const SearchConfig& config) override;
    SearchStatus step(std::size_t iterations = 1) override;
    SearchStatus status() const override {
        return SearchBase::status();
    }
    const SearchSnapshot& snapshot() const override {
        return SearchBase::snapshot();
    }

private:
    struct QueueItem {
        std::int32_t dist;
        std::int32_t idx;
    };

    struct QueueItemCompare {
        bool operator()(const QueueItem& a, const QueueItem& b) const {
            return a.dist > b.dist;
        }
    };

    void rebuildPath(std::int32_t startIdx, std::int32_t goalIdx);

    std::priority_queue<QueueItem, std::vector<QueueItem>, QueueItemCompare> open_;
};

} // namespace pathcore
