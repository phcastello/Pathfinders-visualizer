#include <iostream>

#include "pathcore/AStar.h"
#include "pathcore/Grid.h"
#include "pathcore/NodeState.h"
#include "pathcore/SearchConfig.h"

int main() {
    pathcore::Grid grid(10, 10);
    pathcore::SearchConfig config;
    config.neighborMode = pathcore::NeighborMode::Four;
    config.useWeights = true;
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

    pathcore::AStar astar;
    if (!astar.reset(grid, pathcore::CellPos{0, 0}, pathcore::CellPos{9, 9}, config)) {
        std::cout << "AStar reset failed\n";
        return 1;
    }

    pathcore::SearchStatus status = astar.status();
    std::size_t steps = 0;
    const std::size_t stepLimit = static_cast<std::size_t>(grid.size()) * 10;
    while (status == pathcore::SearchStatus::Running && steps < stepLimit) {
        status = astar.step(1);
        ++steps;
    }

    std::size_t pathCount = 0;
    for (pathcore::NodeState state : astar.snapshot().state) {
        if (state == pathcore::NodeState::Path) {
            ++pathCount;
        }
    }

    const char* statusLabel = "Unknown";
    switch (status) {
    case pathcore::SearchStatus::Found:
        statusLabel = "Found";
        break;
    case pathcore::SearchStatus::NoPath:
        statusLabel = "NoPath";
        break;
    case pathcore::SearchStatus::Running:
        statusLabel = "Running";
        break;
    case pathcore::SearchStatus::NotStarted:
        statusLabel = "NotStarted";
        break;
    }

    std::cout << "AStar status=" << statusLabel << " steps=" << steps
              << " pathCount=" << pathCount << "\n";

    return 0;
}
