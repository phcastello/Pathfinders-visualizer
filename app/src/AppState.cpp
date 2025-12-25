#include "AppState.h"

#include "pathcore/AStar.h"
#include "pathcore/Dijkstra.h"

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

void AppState::setAlgorithm(AlgorithmKind kind) {
    if (algorithm_ == kind) {
        return;
    }
    algorithm_ = kind;
    search_.reset();
    resetSearch();
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

    grid_.clearBlocked();

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
