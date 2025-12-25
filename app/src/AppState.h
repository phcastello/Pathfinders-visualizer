#pragma once

#include <memory>

#include "pathcore/Grid.h"
#include "pathcore/ISearch.h"
#include "pathcore/SearchConfig.h"
#include "pathcore/SearchSnapshot.h"
#include "pathcore/SearchStatus.h"
#include "pathcore/Types.h"

class AppState {
public:
    enum class AlgorithmKind {
        Dijkstra,
        AStar
    };

    AppState();

    const pathcore::Grid& grid() const;
    const pathcore::SearchSnapshot* snapshot() const;
    pathcore::SearchStatus status() const;
    AlgorithmKind algorithm() const;
    bool playing() const;
    pathcore::CellPos start() const;
    pathcore::CellPos goal() const;

    void setAlgorithm(AlgorithmKind kind);
    void togglePlay();
    void pause();
    void stepOnce();
    void resetSearch();
    void tick();

private:
    void buildHardcodedMap();
    void createSearchIfNeeded();

    pathcore::Grid grid_;
    pathcore::CellPos start_{};
    pathcore::CellPos goal_{};
    pathcore::SearchConfig config_{};
    AlgorithmKind algorithm_{AlgorithmKind::Dijkstra};
    std::unique_ptr<pathcore::ISearch> search_;
    bool playing_{false};
    int stepsPerTick_{5};
};
