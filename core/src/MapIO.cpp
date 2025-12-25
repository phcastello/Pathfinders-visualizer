#include "pathcore/MapIO.h"

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

namespace pathcore {
namespace {

bool setError(MapIoError* err, const std::string& message) {
    if (err) {
        err->message = message;
    }
    return false;
}

std::optional<LoadedMap> failLoad(MapIoError* err, const std::string& message) {
    if (err) {
        err->message = message;
    }
    return std::nullopt;
}

bool parseIntPair(const std::string& line, int* a, int* b) {
    if (!a || !b) {
        return false;
    }
    std::istringstream iss(line);
    if (!(iss >> *a >> *b)) {
        return false;
    }
    std::string extra;
    if (iss >> extra) {
        return false;
    }
    return true;
}

bool parseTokenToInt(const std::string& token, std::int64_t* value) {
    if (!value) {
        return false;
    }
    std::istringstream iss(token);
    if (!(iss >> *value)) {
        return false;
    }
    char extra = '\0';
    if (iss >> extra) {
        return false;
    }
    return true;
}

int clampCost(std::int64_t value) {
    if (value < 1) {
        return 1;
    }
    if (value > 10) {
        return 10;
    }
    return static_cast<int>(value);
}

bool hasNonWhitespace(const std::string& line) {
    return line.find_first_not_of(" \t\r\n") != std::string::npos;
}

} // namespace

bool saveMapToFile(const Grid& grid, CellPos start, CellPos goal, const std::string& filePath,
                   MapIoError* err) {
    if (filePath.empty()) {
        return setError(err, "Missing file path.");
    }
    if (grid.width() <= 0 || grid.height() <= 0) {
        return setError(err, "Grid has invalid dimensions.");
    }
    if (!grid.inBounds(start) || !grid.inBounds(goal)) {
        return setError(err, "Start or goal is out of bounds.");
    }
    if (start == goal) {
        return setError(err, "Start and goal must be different.");
    }
    if (grid.isBlocked(start) || grid.isBlocked(goal)) {
        return setError(err, "Start or goal is blocked.");
    }

    std::ofstream out(filePath);
    if (!out) {
        return setError(err, "Failed to open file for writing.");
    }

    out << "PATHVIZ 1\n";
    out << grid.width() << ' ' << grid.height() << '\n';
    out << start.x << ' ' << start.y << '\n';
    out << goal.x << ' ' << goal.y << '\n';

    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            if (x > 0) {
                out << ' ';
            }
            const CellPos pos{x, y};
            if (grid.isBlocked(pos)) {
                out << '#';
            } else {
                const int cost = clampCost(grid.cost(pos));
                out << cost;
            }
        }
        out << '\n';
    }

    if (!out) {
        return setError(err, "Failed while writing map data.");
    }

    return true;
}

std::optional<LoadedMap> loadMapFromFile(const std::string& filePath, MapIoError* err) {
    if (filePath.empty()) {
        return failLoad(err, "Missing file path.");
    }

    std::ifstream in(filePath);
    if (!in) {
        return failLoad(err, "Failed to open file for reading.");
    }

    std::string line;
    if (!std::getline(in, line)) {
        return failLoad(err, "Missing header line.");
    }

    {
        std::istringstream header(line);
        std::string magic;
        int version = 0;
        if (!(header >> magic >> version) || magic != "PATHVIZ" || version != 1) {
            return failLoad(err, "Invalid header (expected 'PATHVIZ 1').");
        }
        std::string extra;
        if (header >> extra) {
            return failLoad(err, "Unexpected data after header.");
        }
    }

    if (!std::getline(in, line)) {
        return failLoad(err, "Missing grid size line.");
    }
    int width = 0;
    int height = 0;
    if (!parseIntPair(line, &width, &height)) {
        return failLoad(err, "Invalid grid size line.");
    }
    if (width <= 0 || height <= 0) {
        return failLoad(err, "Grid dimensions must be positive.");
    }

    if (!std::getline(in, line)) {
        return failLoad(err, "Missing start position line.");
    }
    int startX = 0;
    int startY = 0;
    if (!parseIntPair(line, &startX, &startY)) {
        return failLoad(err, "Invalid start position line.");
    }

    if (!std::getline(in, line)) {
        return failLoad(err, "Missing goal position line.");
    }
    int goalX = 0;
    int goalY = 0;
    if (!parseIntPair(line, &goalX, &goalY)) {
        return failLoad(err, "Invalid goal position line.");
    }

    CellPos start{startX, startY};
    CellPos goal{goalX, goalY};
    if (!pathcore::inBounds(width, height, start) || !pathcore::inBounds(width, height, goal)) {
        return failLoad(err, "Start or goal is out of bounds.");
    }
    if (start == goal) {
        return failLoad(err, "Start and goal must be different.");
    }

    Grid grid(width, height, 1);
    for (int y = 0; y < height; ++y) {
        if (!std::getline(in, line)) {
            return failLoad(err, "Unexpected end of file while reading grid data.");
        }
        std::istringstream row(line);
        for (int x = 0; x < width; ++x) {
            std::string token;
            if (!(row >> token)) {
                return failLoad(err, "Not enough cells in row " + std::to_string(y) + ".");
            }
            const CellPos pos{x, y};
            if (token == "#") {
                grid.setBlocked(pos, true);
                grid.setCost(pos, 1);
            } else {
                std::int64_t value = 0;
                if (!parseTokenToInt(token, &value)) {
                    return failLoad(err, "Invalid cell token at (" + std::to_string(x) + ", " +
                                               std::to_string(y) + ").");
                }
                grid.setBlocked(pos, false);
                grid.setCost(pos, clampCost(value));
            }
        }
        std::string extra;
        if (row >> extra) {
            return failLoad(err, "Too many cells in row " + std::to_string(y) + ".");
        }
    }

    while (std::getline(in, line)) {
        if (hasNonWhitespace(line)) {
            return failLoad(err, "Unexpected extra data after grid.");
        }
    }

    return LoadedMap{grid, start, goal};
}

} // namespace pathcore
