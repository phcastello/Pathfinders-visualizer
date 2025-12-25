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
};

} // namespace pathcore
