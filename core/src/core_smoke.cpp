#include <iostream>

#include "pathcore/Grid.h"

int main() {
    pathcore::Grid grid(10, 10);
    grid.setBlocked(pathcore::CellPos{3, 3}, true);
    grid.setBlocked(pathcore::CellPos{4, 3}, true);
    grid.fillCost(2);
    grid.setCost(pathcore::CellPos{5, 5}, 3);

    std::cout << "Grid " << grid.width() << "x" << grid.height() << " size=" << grid.size() << "\n";
    std::cout << "Cell(3,3) blocked="
              << (grid.isBlocked(pathcore::CellPos{3, 3}) ? "true" : "false") << "\n";
    std::cout << "Cell(5,5) cost=" << grid.cost(pathcore::CellPos{5, 5}) << "\n";
    std::cout << "Neighbors(5,5)="
              << grid.neighbors4(pathcore::CellPos{5, 5}).size() << "\n";

    return 0;
}
