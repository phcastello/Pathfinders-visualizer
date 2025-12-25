#pragma once

#include <optional>
#include <QMouseEvent>
#include <QWidget>

#include "pathcore/Types.h"

class AppState;

class GridView : public QWidget {
public:
    explicit GridView(QWidget* parent = nullptr);

    void setAppState(AppState* state);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    struct GridLayout {
        int gridWidth{0};
        int gridHeight{0};
        qreal cellSize{0.0};
        qreal offsetX{0.0};
        qreal offsetY{0.0};
    };

    std::optional<GridLayout> computeLayout() const;
    std::optional<pathcore::CellPos> cellFromMousePos(const QPoint& pos) const;
    void applyToolAt(pathcore::CellPos cell, Qt::MouseButton button);

    AppState* state_{nullptr};
};
