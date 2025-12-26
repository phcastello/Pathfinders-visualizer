#pragma once

#include <QWidget>

class AppState;
class GridView;
class QLabel;
class QVBoxLayout;

class VersusView : public QWidget {
public:
    explicit VersusView(QWidget* parent = nullptr);

    void setStates(AppState* left, AppState* right);
    GridView* leftGrid() const;
    GridView* rightGrid() const;
    QWidget* leftControlsWidget() const;
    QWidget* rightControlsWidget() const;
    QVBoxLayout* leftControlsLayout() const;
    QVBoxLayout* rightControlsLayout() const;
    void refreshHeaders();

private:
    AppState* leftState_{nullptr};
    AppState* rightState_{nullptr};
    QLabel* leftHeader_{nullptr};
    QLabel* rightHeader_{nullptr};
    GridView* leftGrid_{nullptr};
    GridView* rightGrid_{nullptr};
    QWidget* leftControls_{nullptr};
    QWidget* rightControls_{nullptr};
    QVBoxLayout* leftControlsLayout_{nullptr};
    QVBoxLayout* rightControlsLayout_{nullptr};
};
