#include "pathcore/Grid.h"

#include <cassert>
#include <cstddef>

namespace pathcore {

Grid::Grid(int width, int height, int defaultCost)
    : width_(width), height_(height) {
    if (width_ < 0 || height_ < 0) {
        assert(false && "Grid dimensions must be non-negative");
        width_ = 0;
        height_ = 0;
    }
    if (defaultCost < 1) {
        assert(false && "Grid default cost must be >= 1");
        defaultCost = 1;
    }

    const std::size_t total = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
    cells_.assign(total, Cell{false, defaultCost});
}

int Grid::width() const {
    return width_;
}

int Grid::height() const {
    return height_;
}

int Grid::size() const {
    return width_ * height_;
}

bool Grid::inBounds(CellPos p) const {
    return pathcore::inBounds(width_, height_, p);
}

const Cell& Grid::cell(CellPos p) const {
    assert(inBounds(p));
    return cells_[static_cast<std::size_t>(toIndex(width_, p))];
}

Cell& Grid::cell(CellPos p) {
    assert(inBounds(p));
    return cells_[static_cast<std::size_t>(toIndex(width_, p))];
}

bool Grid::setBlocked(CellPos p, bool blocked) {
    if (!inBounds(p)) {
        return false;
    }
    cells_[static_cast<std::size_t>(toIndex(width_, p))].blocked = blocked;
    return true;
}

bool Grid::isBlocked(CellPos p) const {
    if (!inBounds(p)) {
        assert(false && "CellPos out of bounds");
        return true;
    }
    return cells_[static_cast<std::size_t>(toIndex(width_, p))].blocked;
}

bool Grid::setCost(CellPos p, int cost) {
    if (!inBounds(p) || cost < 1) {
        return false;
    }
    cells_[static_cast<std::size_t>(toIndex(width_, p))].cost = cost;
    return true;
}

int Grid::cost(CellPos p) const {
    if (!inBounds(p)) {
        assert(false && "CellPos out of bounds");
        return 1;
    }
    return cells_[static_cast<std::size_t>(toIndex(width_, p))].cost;
}

void Grid::clearBlocked() {
    for (auto& cell : cells_) {
        cell.blocked = false;
    }
}

void Grid::fillCost(int cost) {
    if (cost < 1) {
        assert(false && "Cost must be >= 1");
        return;
    }
    for (auto& cell : cells_) {
        cell.cost = cost;
    }
}

std::vector<CellPos> Grid::neighbors4(CellPos p) const {
    std::vector<CellPos> result;
    result.reserve(4);

    if (!inBounds(p)) {
        return result;
    }

    CellPos right{p.x + 1, p.y};
    if (inBounds(right) && !isBlocked(right)) {
        result.push_back(right);
    }

    CellPos left{p.x - 1, p.y};
    if (inBounds(left) && !isBlocked(left)) {
        result.push_back(left);
    }

    CellPos down{p.x, p.y + 1};
    if (inBounds(down) && !isBlocked(down)) {
        result.push_back(down);
    }

    CellPos up{p.x, p.y - 1};
    if (inBounds(up) && !isBlocked(up)) {
        result.push_back(up);
    }

    return result;
}

std::vector<CellPos> Grid::neighbors8(CellPos p) const {
    std::vector<CellPos> result;
    result.reserve(8);

    if (!inBounds(p)) {
        return result;
    }

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            CellPos neighbor{p.x + dx, p.y + dy};
            if (inBounds(neighbor) && !isBlocked(neighbor)) {
                result.push_back(neighbor);
            }
        }
    }

    return result;
}

} // namespace pathcore
