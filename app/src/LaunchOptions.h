#pragma once

enum class AppMode {
    Single,
    Versus
};

enum class AlgoKind {
    Dijkstra,
    AStar
};

struct LaunchOptions {
    AppMode mode{AppMode::Single};
    AlgoKind singleAlgo{AlgoKind::Dijkstra};
    AlgoKind leftAlgo{AlgoKind::Dijkstra};
    AlgoKind rightAlgo{AlgoKind::AStar};
};
