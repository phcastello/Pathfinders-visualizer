#pragma once

#include <QMainWindow>

#include "AppState.h"

class QAction;
class GridView;
class QTimer;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void updateStatusBar();
    void updatePlayAction();

    AppState appState_;
    GridView* gridView_{nullptr};
    QTimer* timer_{nullptr};
    QAction* playAction_{nullptr};
    QAction* stepAction_{nullptr};
    QAction* resetAction_{nullptr};
    QAction* dijkstraAction_{nullptr};
    QAction* aStarAction_{nullptr};
};
