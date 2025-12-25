#pragma once

#include <cstdint>

namespace pathcore {

enum class NeighborMode : std::uint8_t {
    Four = 0,
    Eight
};

struct SearchConfig {
    NeighborMode neighborMode{NeighborMode::Four};
    bool useWeights{false};
    bool allowCornerCutting{false};
    bool penalizeTurns{false};
    int turnPenalty{1};
};

} // namespace pathcore
