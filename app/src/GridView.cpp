#include "GridView.h"

#include <cmath>
#include <QPainter>

#include "AppState.h"
#include "pathcore/NodeState.h"

GridView::GridView(QWidget* parent)
    : QWidget(parent) {}

void GridView::setAppState(AppState* state) {
    state_ = state;
    update();
}

std::optional<GridView::GridLayout> GridView::computeLayout() const {
    if (!state_) {
        return std::nullopt;
    }

    const pathcore::Grid& grid = state_->grid();
    const int gridWidth = grid.width();
    const int gridHeight = grid.height();

    if (gridWidth <= 0 || gridHeight <= 0) {
        return std::nullopt;
    }

    const qreal cellSize =
        qMin(width() / static_cast<qreal>(gridWidth), height() / static_cast<qreal>(gridHeight));
    if (cellSize <= 0.0) {
        return std::nullopt;
    }

    const qreal totalWidth = cellSize * gridWidth;
    const qreal totalHeight = cellSize * gridHeight;
    const qreal offsetX = (width() - totalWidth) * 0.5;
    const qreal offsetY = (height() - totalHeight) * 0.5;

    return GridLayout{gridWidth, gridHeight, cellSize, offsetX, offsetY};
}

std::optional<pathcore::CellPos> GridView::cellFromMousePos(const QPoint& pos) const {
    const auto layout = computeLayout();
    if (!layout) {
        return std::nullopt;
    }

    const qreal localX = pos.x() - layout->offsetX;
    const qreal localY = pos.y() - layout->offsetY;
    if (localX < 0.0 || localY < 0.0) {
        return std::nullopt;
    }

    const int x = static_cast<int>(std::floor(localX / layout->cellSize));
    const int y = static_cast<int>(std::floor(localY / layout->cellSize));

    if (x < 0 || y < 0 || x >= layout->gridWidth || y >= layout->gridHeight) {
        return std::nullopt;
    }

    return pathcore::CellPos{x, y};
}

void GridView::applyToolAt(pathcore::CellPos cell, Qt::MouseButton button) {
    if (!state_) {
        return;
    }

    if (button != Qt::LeftButton) {
        return;
    }

    switch (state_->tool()) {
    case AppState::EditTool::DrawWall:
        state_->applyWallAt(cell, true);
        break;
    case AppState::EditTool::EraseWall:
        state_->applyWallAt(cell, false);
        break;
    case AppState::EditTool::SetStart:
        state_->setStartAt(cell);
        break;
    case AppState::EditTool::SetGoal:
        state_->setGoalAt(cell);
        break;
    case AppState::EditTool::PaintCost:
        state_->applyCostAt(cell, state_->paintCost());
        break;
    }
}

void GridView::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const QColor backgroundColor(243, 246, 251);
    painter.fillRect(rect(), backgroundColor);

    if (!state_) {
        return;
    }

    const pathcore::Grid& grid = state_->grid();
    const auto layout = computeLayout();
    if (!layout) {
        return;
    }

    const int gridWidth = layout->gridWidth;
    const int gridHeight = layout->gridHeight;
    const qreal cellSize = layout->cellSize;
    const qreal offsetX = layout->offsetX;
    const qreal offsetY = layout->offsetY;

    const pathcore::SearchSnapshot* snapshot = state_->snapshot();
    const bool snapshotValid = snapshot && snapshot->valid();

    const pathcore::CellPos start = state_->start();
    const pathcore::CellPos goal = state_->goal();

    const QColor wallColor(15, 23, 42);
    const QColor unseenColor(248, 250, 252);
    const QColor openColor(52, 211, 153);
    const QColor closedColor(96, 165, 250);
    const QColor pathColor(251, 191, 36);
    const QColor startColor(34, 197, 94);
    const QColor goalColor(239, 68, 68);

    auto costShade = [](int cost) {
        if (cost < 1) {
            cost = 1;
        } else if (cost > 10) {
            cost = 10;
        }
        const int base = 242;
        const int range = 55;
        const int value = base - ((cost - 1) * range) / 9;
        return QColor(value, value, value);
    };

    const bool showCosts = state_->useWeights();
    const bool drawGridLines = cellSize >= 6.0;
    if (drawGridLines) {
        painter.setPen(QPen(QColor(214, 222, 232)));
    } else {
        painter.setPen(Qt::NoPen);
    }

    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            const pathcore::CellPos pos{x, y};
            QColor cellColor = unseenColor;

            if (grid.isBlocked(pos)) {
                cellColor = wallColor;
            } else {
                const pathcore::NodeState state =
                    snapshotValid ? snapshot->getState(pos) : pathcore::NodeState::Unseen;
                if (snapshotValid && state != pathcore::NodeState::Unseen) {
                    switch (state) {
                    case pathcore::NodeState::Path:
                        cellColor = pathColor;
                        break;
                    case pathcore::NodeState::Closed:
                        cellColor = closedColor;
                        break;
                    case pathcore::NodeState::Open:
                        cellColor = openColor;
                        break;
                    case pathcore::NodeState::Unseen:
                    default:
                        cellColor = unseenColor;
                        break;
                    }
                } else if (showCosts) {
                    cellColor = costShade(grid.cost(pos));
                } else {
                    cellColor = unseenColor;
                }
            }

            if (pos == start) {
                cellColor = startColor;
            } else if (pos == goal) {
                cellColor = goalColor;
            }

            const QRectF cellRect(offsetX + x * cellSize, offsetY + y * cellSize, cellSize, cellSize);
            painter.fillRect(cellRect, cellColor);
            if (drawGridLines) {
                painter.drawRect(cellRect);
            }
        }
    }
}

void GridView::mousePressEvent(QMouseEvent* event) {
    if (!event) {
        return;
    }
    const auto cell = cellFromMousePos(event->pos());
    if (!cell) {
        return;
    }
    applyToolAt(*cell, event->button());
    update();
}

void GridView::mouseMoveEvent(QMouseEvent* event) {
    if (!event) {
        return;
    }
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    const auto cell = cellFromMousePos(event->pos());
    if (!cell) {
        return;
    }
    applyToolAt(*cell, Qt::LeftButton);
    update();
}

void GridView::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
}
