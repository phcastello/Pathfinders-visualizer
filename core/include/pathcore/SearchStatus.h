#pragma once

#include <cstdint>

namespace pathcore {

enum class SearchStatus : std::uint8_t {
    NotStarted = 0,
    Running,
    Found,
    NoPath
};

} // namespace pathcore
