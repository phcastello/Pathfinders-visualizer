#pragma once

#include <QMainWindow>

#include "AppState.h"
#include "LaunchOptions.h"

class QAction;
class GridView;
class QSpinBox;
class QTimer;
class VersusView;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(const LaunchOptions& opts, QWidget* parent = nullptr);

private:
    bool isVersus() const;
    bool showResizeGridDialog();
    void syncRightFromLeft();
    void tickCurrentMode();
    void updateViewsCurrentMode();
    void updateStatusBarCurrentMode();
    void updateStatusBarVersus();
    void updateStatusBar();
    void updatePlayAction();

    LaunchOptions options_;
    AppState appState_;
    AppState leftState_;
    AppState rightState_;
    GridView* gridView_{nullptr};
    VersusView* versusView_{nullptr};
    QTimer* timer_{nullptr};
    QAction* playAction_{nullptr};
    QAction* stepAction_{nullptr};
    QAction* resetAction_{nullptr};
    QAction* weightsAction_{nullptr};
    QAction* turnPenaltyAction_{nullptr};
    QSpinBox* turnPenaltySpin_{nullptr};
    QAction* leftWeightsAction_{nullptr};
    QAction* rightWeightsAction_{nullptr};
    QAction* leftDiagonalAction_{nullptr};
    QAction* rightDiagonalAction_{nullptr};
    QAction* leftCornerAction_{nullptr};
    QAction* rightCornerAction_{nullptr};
    QAction* leftTurnPenaltyAction_{nullptr};
    QAction* rightTurnPenaltyAction_{nullptr};
    QSpinBox* leftTurnPenaltySpin_{nullptr};
    QSpinBox* rightTurnPenaltySpin_{nullptr};
    QSpinBox* leftSpeedSpin_{nullptr};
    QSpinBox* rightSpeedSpin_{nullptr};
    QAction* dijkstraAction_{nullptr};
    QAction* aStarAction_{nullptr};
};
