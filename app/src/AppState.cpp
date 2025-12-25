#include "AppState.h"

#include "pathcore/AStar.h"
#include "pathcore/Dijkstra.h"
#include "pathcore/MapIO.h"

AppState::AppState()
    : grid_(40, 25, 1) {
    buildHardcodedMap();
    resetSearch();
}

const pathcore::Grid& AppState::grid() const {
    return grid_;
}

const pathcore::SearchSnapshot* AppState::snapshot() const {
    if (!search_) {
        return nullptr;
    }
    return &search_->snapshot();
}

pathcore::SearchStatus AppState::status() const {
    if (!search_) {
        return pathcore::SearchStatus::NotStarted;
    }
    return search_->status();
}

AppState::AlgorithmKind AppState::algorithm() const {
    return algorithm_;
}

bool AppState::playing() const {
    return playing_;
}

pathcore::CellPos AppState::start() const {
    return start_;
}

pathcore::CellPos AppState::goal() const {
    return goal_;
}

AppState::EditTool AppState::tool() const {
    return tool_;
}

int AppState::paintCost() const {
    return paintCost_;
}

bool AppState::useWeights() const {
    return config_.useWeights;
}

pathcore::NeighborMode AppState::neighborMode() const {
    return config_.neighborMode;
}

bool AppState::allowCornerCutting() const {
    return config_.allowCornerCutting;
}

int AppState::stepsPerTick() const {
    return stepsPerTick_;
}

void AppState::setAlgorithm(AlgorithmKind kind) {
    if (algorithm_ == kind) {
        return;
    }
    algorithm_ = kind;
    search_.reset();
    resetSearch();
}

void AppState::setTool(EditTool tool) {
    tool_ = tool;
}

void AppState::setPaintCost(int c) {
    if (c < 1) {
        c = 1;
    } else if (c > 10) {
        c = 10;
    }
    if (paintCost_ == c) {
        return;
    }
    paintCost_ = c;
}

void AppState::setUseWeights(bool enabled) {
    if (config_.useWeights == enabled) {
        return;
    }
    config_.useWeights = enabled;
    pause();
    resetSearch();
}

void AppState::setNeighborMode(pathcore::NeighborMode mode) {
    if (config_.neighborMode == mode) {
        return;
    }
    config_.neighborMode = mode;
    pause();
    resetSearch();
}

void AppState::setCornerCutting(bool enabled) {
    if (config_.allowCornerCutting == enabled) {
        return;
    }
    config_.allowCornerCutting = enabled;
    pause();
    resetSearch();
}

void AppState::setStepsPerTick(int v) {
    if (v < 1) {
        v = 1;
    } else if (v > 200) {
        v = 200;
    }
    if (stepsPerTick_ == v) {
        return;
    }
    stepsPerTick_ = v;
}

void AppState::togglePlay() {
    if (playing_) {
        playing_ = false;
        return;
    }
    if (!search_) {
        resetSearch();
    }
    if (search_ && search_->status() == pathcore::SearchStatus::Running) {
        playing_ = true;
    }
}

void AppState::pause() {
    playing_ = false;
}

void AppState::stepOnce() {
    if (!search_) {
        resetSearch();
    }
    if (!search_ || search_->status() != pathcore::SearchStatus::Running) {
        return;
    }
    search_->step(1);
}

void AppState::resetSearch() {
    createSearchIfNeeded();
    if (!search_) {
        return;
    }
    search_->reset(grid_, start_, goal_, config_);
    playing_ = false;
}

bool AppState::applyWallAt(pathcore::CellPos p, bool blocked) {
    if (!grid_.inBounds(p)) {
        return false;
    }
    if (blocked && (p == start_ || p == goal_)) {
        return false;
    }
    if (grid_.isBlocked(p) == blocked) {
        return false;
    }
    grid_.setBlocked(p, blocked);
    pause();
    resetSearch();
    return true;
}

bool AppState::applyCostAt(pathcore::CellPos p, int cost) {
    if (!grid_.inBounds(p) || cost < 1 || cost > 10) {
        return false;
    }
    if (grid_.cost(p) == cost) {
        return false;
    }
    if (!grid_.setCost(p, cost)) {
        return false;
    }
    if (!config_.useWeights) {
        config_.useWeights = true;
    }
    pause();
    resetSearch();
    return true;
}

bool AppState::setStartAt(pathcore::CellPos p) {
    if (!grid_.inBounds(p)) {
        return false;
    }
    if (p == goal_) {
        return false;
    }

    bool changed = false;
    if (grid_.isBlocked(p)) {
        grid_.setBlocked(p, false);
        changed = true;
    }
    if (start_ != p) {
        start_ = p;
        changed = true;
    }
    if (changed) {
        pause();
        resetSearch();
    }
    return changed;
}

bool AppState::setGoalAt(pathcore::CellPos p) {
    if (!grid_.inBounds(p)) {
        return false;
    }
    if (p == start_) {
        return false;
    }

    bool changed = false;
    if (grid_.isBlocked(p)) {
        grid_.setBlocked(p, false);
        changed = true;
    }
    if (goal_ != p) {
        goal_ = p;
        changed = true;
    }
    if (changed) {
        pause();
        resetSearch();
    }
    return changed;
}

void AppState::clearWalls() {
    grid_.clearBlocked();
    grid_.fillCost(1);
    config_.useWeights = false;
    pause();
    resetSearch();
}

bool AppState::saveMap(const std::string& path, std::string* err) const {
    pathcore::MapIoError ioErr;
    if (!pathcore::saveMapToFile(grid_, start_, goal_, path, &ioErr)) {
        if (err) {
            *err = ioErr.message;
        }
        return false;
    }
    return true;
}

bool AppState::loadMap(const std::string& path, std::string* err) {
    pathcore::MapIoError ioErr;
    std::optional<pathcore::LoadedMap> loaded = pathcore::loadMapFromFile(path, &ioErr);
    if (!loaded) {
        if (err) {
            *err = ioErr.message;
        }
        return false;
    }

    grid_ = loaded->grid;
    start_ = loaded->start;
    goal_ = loaded->goal;

    if (grid_.inBounds(start_) && grid_.isBlocked(start_)) {
        grid_.setBlocked(start_, false);
    }
    if (grid_.inBounds(goal_) && grid_.isBlocked(goal_)) {
        grid_.setBlocked(goal_, false);
    }

    bool hasWeights = false;
    for (int y = 0; y < grid_.height(); ++y) {
        for (int x = 0; x < grid_.width(); ++x) {
            if (grid_.cost(pathcore::CellPos{x, y}) > 1) {
                hasWeights = true;
                break;
            }
        }
        if (hasWeights) {
            break;
        }
    }
    config_.useWeights = hasWeights;

    pause();
    resetSearch();
    return true;
}

void AppState::newMap() {
    grid_.clearBlocked();
    grid_.fillCost(1);
    config_.useWeights = false;

    const int width = grid_.width();
    const int height = grid_.height();

    auto clampCoord = [](int value, int minValue, int maxValue) {
        if (value < minValue) {
            return minValue;
        }
        if (value > maxValue) {
            return maxValue;
        }
        return value;
    };

    if (width <= 0 || height <= 0) {
        start_ = pathcore::CellPos{0, 0};
        goal_ = pathcore::CellPos{0, 0};
    } else {
        start_.x = clampCoord(1, 0, width - 1);
        start_.y = clampCoord(1, 0, height - 1);
        goal_.x = clampCoord(width - 2, 0, width - 1);
        goal_.y = clampCoord(height - 2, 0, height - 1);
    }

    if (start_ == goal_) {
        if (width > 1) {
            goal_.x = (start_.x == 0) ? 1 : 0;
        } else if (height > 1) {
            goal_.y = (start_.y == 0) ? 1 : 0;
        }
    }

    paintCost_ = 5;
    pause();
    resetSearch();
}

void AppState::tick() {
    if (!playing_) {
        return;
    }
    if (!search_) {
        resetSearch();
    }
    if (!search_ || search_->status() != pathcore::SearchStatus::Running) {
        playing_ = false;
        return;
    }
    search_->step(static_cast<std::size_t>(stepsPerTick_));
    if (search_->status() != pathcore::SearchStatus::Running) {
        playing_ = false;
    }
}

void AppState::buildHardcodedMap() {
    config_.neighborMode = pathcore::NeighborMode::Four;
    config_.useWeights = false;
    config_.allowCornerCutting = false;
    paintCost_ = 5;

    grid_.clearBlocked();
    grid_.fillCost(1);

    start_ = pathcore::CellPos{2, 2};
    goal_ = pathcore::CellPos{37, 22};

    for (int y = 2; y < 23; ++y) {
        if (y >= 11 && y <= 13) {
            continue;
        }
        grid_.setBlocked(pathcore::CellPos{10, y}, true);
    }

    for (int x = 10; x < 32; ++x) {
        if (x >= 20 && x <= 22) {
            continue;
        }
        grid_.setBlocked(pathcore::CellPos{x, 8}, true);
    }

    for (int y = 14; y < 19; ++y) {
        for (int x = 24; x < 31; ++x) {
            if (x == 27 && y == 16) {
                continue;
            }
            grid_.setBlocked(pathcore::CellPos{x, y}, true);
        }
    }

    for (int y = 3; y < 12; ++y) {
        if (y == 6) {
            continue;
        }
        grid_.setBlocked(pathcore::CellPos{20, y}, true);
    }
}

void AppState::createSearchIfNeeded() {
    if (search_) {
        return;
    }
    switch (algorithm_) {
    case AlgorithmKind::Dijkstra:
        search_ = std::make_unique<pathcore::Dijkstra>();
        break;
    case AlgorithmKind::AStar:
        search_ = std::make_unique<pathcore::AStar>();
        break;
    }
}
