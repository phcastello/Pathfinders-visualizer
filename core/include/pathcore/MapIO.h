#pragma once

#include <optional>
#include <string>

#include "pathcore/Grid.h"
#include "pathcore/Types.h"

namespace pathcore {

struct LoadedMap {
    Grid grid;
    CellPos start;
    CellPos goal;
};

struct MapIoError {
    std::string message;
};

bool saveMapToFile(const Grid& grid, CellPos start, CellPos goal, const std::string& filePath,
                   MapIoError* err = nullptr);

std::optional<LoadedMap> loadMapFromFile(const std::string& filePath, MapIoError* err = nullptr);

} // namespace pathcore
