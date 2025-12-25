#pragma once

#include <cstdint>

namespace pathcore {

enum class NodeState : std::uint8_t {
    Unseen = 0,
    Open,
    Closed,
    Path
};

} // namespace pathcore
