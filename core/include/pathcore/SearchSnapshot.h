#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include "pathcore/NodeState.h"
#include "pathcore/Types.h"

namespace pathcore {

struct SearchSnapshot {
    int width{0};
    int height{0};

    std::vector<NodeState> state;
    std::vector<std::int32_t> parent;
    std::vector<std::int32_t> gScore;
    std::vector<std::int32_t> fScore;

    static constexpr std::int32_t kNoParent = -1;
    static constexpr std::int32_t kInfScore = 1'000'000'000;

    int size() const {
        return width * height;
    }

    bool valid() const {
        if (width <= 0 || height <= 0) {
            return false;
        }
        const std::size_t expected = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
        return state.size() == expected && parent.size() == expected && gScore.size() == expected
            && fScore.size() == expected;
    }

    void resize(int w, int h) {
        width = w;
        height = h;
        std::size_t total = 0;
        if (w > 0 && h > 0) {
            total = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
        }
        state.resize(total);
        parent.resize(total);
        gScore.resize(total);
        fScore.resize(total);
        clear();
    }

    void clear() {
        for (auto& value : state) {
            value = NodeState::Unseen;
        }
        for (auto& value : parent) {
            value = kNoParent;
        }
        for (auto& value : gScore) {
            value = kInfScore;
        }
        for (auto& value : fScore) {
            value = kInfScore;
        }
    }

    bool inBounds(CellPos p) const {
        return pathcore::inBounds(width, height, p);
    }

    std::int32_t indexOf(CellPos p) const {
        if (!inBounds(p)) {
            return -1;
        }
        return static_cast<std::int32_t>(pathcore::toIndex(width, p));
    }

    NodeState getState(CellPos p) const {
        if (!inBounds(p)) {
            return NodeState::Unseen;
        }
        const std::size_t idx = static_cast<std::size_t>(pathcore::toIndex(width, p));
        return state[idx];
    }

    bool setState(CellPos p, NodeState s) {
        if (!inBounds(p)) {
            return false;
        }
        const std::size_t idx = static_cast<std::size_t>(pathcore::toIndex(width, p));
        state[idx] = s;
        return true;
    }

    bool setParent(CellPos p, std::int32_t parentIndex) {
        if (!inBounds(p)) {
            return false;
        }
        const std::size_t idx = static_cast<std::size_t>(pathcore::toIndex(width, p));
        parent[idx] = parentIndex;
        return true;
    }
};

} // namespace pathcore
