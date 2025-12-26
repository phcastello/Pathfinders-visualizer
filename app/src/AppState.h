#pragma once

#include <cstdint>
#include <memory>
#include <string>

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

    enum class EditTool {
        DrawWall,
        EraseWall,
        SetStart,
        SetGoal,
        PaintCost
    };

    AppState();

    const pathcore::Grid& grid() const;
    int gridWidth() const;
    int gridHeight() const;
    const pathcore::SearchSnapshot* snapshot() const;
    pathcore::SearchStatus status() const;
    AlgorithmKind algorithm() const;
    bool playing() const;
    pathcore::CellPos start() const;
    pathcore::CellPos goal() const;
    EditTool tool() const;
    int paintCost() const;
    bool useWeights() const;
    pathcore::NeighborMode neighborMode() const;
    bool allowCornerCutting() const;
    bool penalizeTurns() const;
    int turnPenalty() const;
    int stepsPerTick() const;
    std::uint64_t algoTimeNs() const;
    double algoTimeMs() const;

    void setAlgorithm(AlgorithmKind kind);
    void setTool(EditTool tool);
    void setPaintCost(int c);
    void setUseWeights(bool enabled);
    void setNeighborMode(pathcore::NeighborMode mode);
    void setCornerCutting(bool enabled);
    void setPenalizeTurns(bool enabled);
    void setTurnPenalty(int value);
    void togglePlay();
    void pause();
    void stepOnce();
    void resetSearch();
    void tick();
    void setStepsPerTick(int v);

    bool saveMap(const std::string& path, std::string* err = nullptr) const;
    bool loadMap(const std::string& path, std::string* err = nullptr);
    void newMap();
    bool resizeGrid(int width, int height);

    bool applyWallAt(pathcore::CellPos p, bool blocked);
    bool applyCostAt(pathcore::CellPos p, int cost);
    bool setStartAt(pathcore::CellPos p);
    bool setGoalAt(pathcore::CellPos p);
    void clearWalls();

private:
    void buildHardcodedMap();
    void createSearchIfNeeded();

    pathcore::Grid grid_;
    pathcore::CellPos start_{};
    pathcore::CellPos goal_{};
    pathcore::SearchConfig config_{};
    AlgorithmKind algorithm_{AlgorithmKind::Dijkstra};
    EditTool tool_{EditTool::DrawWall};
    std::unique_ptr<pathcore::ISearch> search_;
    bool playing_{false};
    int stepsPerTick_{5};
    int paintCost_{5};
    std::uint64_t algoTimeNs_{0};
};
