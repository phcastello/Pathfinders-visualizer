#pragma once

#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>

#include "pathcore/ISearch.h"
#include "pathcore/SearchBase.h"

namespace pathcore {

class AStar final : public ISearch, public SearchBase {
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
        std::int32_t f;
        std::int32_t g;
        std::int32_t idx;
    };

    struct QueueItemCompare {
        bool operator()(const QueueItem& a, const QueueItem& b) const {
            if (a.f != b.f) {
                return a.f > b.f;
            }
            return a.g < b.g; // Tie-breaker: prefer larger g to reduce zig-zagging.
        }
    };

    static std::int32_t manhattan(CellPos a, CellPos b);
    void rebuildPath(std::int32_t startIdx, std::int32_t goalIdx);

    std::priority_queue<QueueItem, std::vector<QueueItem>, QueueItemCompare> open_;
};

} // namespace pathcore
