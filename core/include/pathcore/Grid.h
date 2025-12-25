#pragma once

#include <vector>

#include "pathcore/Types.h"

namespace pathcore {

struct Cell {
    bool blocked{false};
    int cost{1};
};

class Grid {
public:
    Grid(int width, int height, int defaultCost = 1);

    int width() const;
    int height() const;
    int size() const;

    bool inBounds(CellPos p) const;

    const Cell& cell(CellPos p) const;
    Cell& cell(CellPos p);

    bool setBlocked(CellPos p, bool blocked);
    bool isBlocked(CellPos p) const;
    bool setCost(CellPos p, int cost);
    int cost(CellPos p) const;

    void clearBlocked();
    void fillCost(int cost);

    std::vector<CellPos> neighbors4(CellPos p) const;

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Cell> cells_;
};

} // namespace pathcore
