#include "GridView.h"

#include <QPainter>

#include "AppState.h"
#include "pathcore/NodeState.h"

GridView::GridView(QWidget* parent)
    : QWidget(parent) {}

void GridView::setAppState(AppState* state) {
    state_ = state;
    update();
}

void GridView::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const QColor backgroundColor(245, 245, 245);
    painter.fillRect(rect(), backgroundColor);

    if (!state_) {
        return;
    }

    const pathcore::Grid& grid = state_->grid();
    const int gridWidth = grid.width();
    const int gridHeight = grid.height();

    if (gridWidth <= 0 || gridHeight <= 0) {
        return;
    }

    const qreal cellSize =
        qMin(width() / static_cast<qreal>(gridWidth), height() / static_cast<qreal>(gridHeight));
    if (cellSize <= 0.0) {
        return;
    }

    const qreal totalWidth = cellSize * gridWidth;
    const qreal totalHeight = cellSize * gridHeight;
    const qreal offsetX = (width() - totalWidth) * 0.5;
    const qreal offsetY = (height() - totalHeight) * 0.5;

    const pathcore::SearchSnapshot* snapshot = state_->snapshot();
    const bool snapshotValid = snapshot && snapshot->valid();

    const pathcore::CellPos start = state_->start();
    const pathcore::CellPos goal = state_->goal();

    const QColor wallColor(30, 30, 30);
    const QColor unseenColor(235, 235, 235);
    const QColor openColor(120, 200, 150);
    const QColor closedColor(95, 140, 210);
    const QColor pathColor(245, 215, 110);
    const QColor startColor(60, 180, 90);
    const QColor goalColor(220, 70, 70);

    const bool drawGridLines = cellSize >= 6.0;
    if (drawGridLines) {
        painter.setPen(QPen(QColor(210, 210, 210)));
    } else {
        painter.setPen(Qt::NoPen);
    }

    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            const pathcore::CellPos pos{x, y};
            QColor cellColor = unseenColor;

            if (grid.isBlocked(pos)) {
                cellColor = wallColor;
            } else if (snapshotValid) {
                switch (snapshot->getState(pos)) {
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
