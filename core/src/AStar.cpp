#include "pathcore/AStar.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "pathcore/Grid.h"
#include "pathcore/NodeState.h"
#include "pathcore/SearchSnapshot.h"
#include "pathcore/Types.h"

namespace pathcore {

std::int32_t AStar::manhattan(CellPos a, CellPos b) {
    return static_cast<std::int32_t>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

bool AStar::reset(const Grid& grid, CellPos start, CellPos goal, const SearchConfig& config) {
    if (!commonReset(grid, start, goal, config)) {
        return false;
    }

    open_ = std::priority_queue<QueueItem, std::vector<QueueItem>, QueueItemCompare>();

    const int width = grid.width();
    const std::int32_t startIdx = static_cast<std::int32_t>(toIndex(width, start));
    const std::size_t startIndex = static_cast<std::size_t>(startIdx);

    const std::int32_t hStart = manhattan(start, goal);
    snapshot_.gScore[startIndex] = 0;
    snapshot_.fScore[startIndex] = hStart;
    snapshot_.parent[startIndex] = SearchSnapshot::kNoParent;
    snapshot_.state[startIndex] = NodeState::Open;
    open_.push(QueueItem{hStart, 0, startIdx});

    return true;
}

SearchStatus AStar::step(std::size_t iterations) {
    if (status_ != SearchStatus::Running) {
        return status_;
    }

    const int width = grid().width();
    const std::int32_t startIdx = static_cast<std::int32_t>(toIndex(width, start_));
    const std::int32_t goalIdx = static_cast<std::int32_t>(toIndex(width, goal_));

    std::size_t expansions = 0;
    while (expansions < iterations) {
        if (open_.empty()) {
            status_ = SearchStatus::NoPath;
            return status_;
        }

        QueueItem current = open_.top();
        open_.pop();

        if (current.idx < 0 || current.idx >= snapshot_.size()) {
            continue;
        }

        const std::size_t idx = static_cast<std::size_t>(current.idx);
        if (snapshot_.state[idx] == NodeState::Closed) {
            continue;
        }
        if (snapshot_.gScore[idx] == SearchSnapshot::kInfScore) {
            continue;
        }
        if (current.f != snapshot_.fScore[idx]) {
            continue;
        }

        snapshot_.state[idx] = NodeState::Closed;
        ++expansions;

        if (current.idx == goalIdx) {
            status_ = SearchStatus::Found;
            rebuildPath(startIdx, goalIdx);
            return status_;
        }

        const CellPos pos = fromIndex(width, current.idx);
        // Ignore diagonal neighbor mode for now; use 4-way neighbors.
        const std::vector<CellPos> neighbors = grid().neighbors4(pos);
        for (const CellPos& neighbor : neighbors) {
            const std::int32_t nIdx = static_cast<std::int32_t>(toIndex(width, neighbor));
            const std::size_t nIndex = static_cast<std::size_t>(nIdx);

            if (snapshot_.state[nIndex] == NodeState::Closed) {
                continue;
            }

            const std::int32_t stepCost =
                config_.useWeights ? static_cast<std::int32_t>(grid().cost(neighbor)) : 1;
            const std::int32_t newG = snapshot_.gScore[idx] + stepCost;

            if (newG < snapshot_.gScore[nIndex]) {
                snapshot_.gScore[nIndex] = newG;
                snapshot_.parent[nIndex] = current.idx;
                const std::int32_t h = manhattan(neighbor, goal_);
                const std::int32_t newF = newG + h;
                snapshot_.fScore[nIndex] = newF;
                snapshot_.state[nIndex] = NodeState::Open;
                open_.push(QueueItem{newF, newG, nIdx});
            }
        }
    }

    return status_;
}

void AStar::rebuildPath(std::int32_t startIdx, std::int32_t goalIdx) {
    std::int32_t cur = goalIdx;
    int steps = 0;
    const int limit = snapshot_.size();

    while (cur != SearchSnapshot::kNoParent && cur != startIdx && steps < limit) {
        snapshot_.state[static_cast<std::size_t>(cur)] = NodeState::Path;
        cur = snapshot_.parent[static_cast<std::size_t>(cur)];
        ++steps;
    }

    if (cur == startIdx) {
        snapshot_.state[static_cast<std::size_t>(startIdx)] = NodeState::Path;
    }

    if (goalIdx >= 0 && goalIdx < snapshot_.size()) {
        snapshot_.state[static_cast<std::size_t>(goalIdx)] = NodeState::Path;
    }
}

} // namespace pathcore
