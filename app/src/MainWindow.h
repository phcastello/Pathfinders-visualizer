#pragma once

#include <QMainWindow>

#include "AppState.h"
#include "LaunchOptions.h"

class QAction;
class GridView;
class QSpinBox;
class QTimer;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(const LaunchOptions& opts, QWidget* parent = nullptr);

private:
    void updateStatusBar();
    void updatePlayAction();

    LaunchOptions options_;
    AppState appState_;
    GridView* gridView_{nullptr};
    QTimer* timer_{nullptr};
    QAction* playAction_{nullptr};
    QAction* stepAction_{nullptr};
    QAction* resetAction_{nullptr};
    QAction* weightsAction_{nullptr};
    QAction* turnPenaltyAction_{nullptr};
    QSpinBox* turnPenaltySpin_{nullptr};
    QAction* dijkstraAction_{nullptr};
    QAction* aStarAction_{nullptr};
};
