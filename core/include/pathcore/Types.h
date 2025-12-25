#pragma once

namespace pathcore {

struct CellPos {
    int x{0};
    int y{0};
};

inline bool operator==(CellPos a, CellPos b) {
    return a.x == b.x && a.y == b.y;
}

inline bool operator!=(CellPos a, CellPos b) {
    return !(a == b);
}

inline int toIndex(int width, CellPos p) {
    return p.y * width + p.x;
}

inline CellPos fromIndex(int width, int idx) {
    if (width <= 0) {
        return CellPos{0, 0};
    }
    return CellPos{idx % width, idx / width};
}

inline bool inBounds(int width, int height, CellPos p) {
    return width > 0 && height > 0 && p.x >= 0 && p.y >= 0 && p.x < width && p.y < height;
}

} // namespace pathcore
