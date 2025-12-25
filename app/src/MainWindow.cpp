#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>

#include "GridView.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("PathViz");
    resize(900, 600);

    gridView_ = new GridView(this);
    gridView_->setAppState(&appState_);
    setCentralWidget(gridView_);

    QToolBar* toolbar = addToolBar("Controls");
    toolbar->setMovable(false);

    playAction_ = toolbar->addAction("Play");
    playAction_->setShortcut(QKeySequence(Qt::Key_Space));

    stepAction_ = toolbar->addAction("Step");
    stepAction_->setShortcuts({QKeySequence(Qt::Key_Return), QKeySequence(Qt::Key_Enter)});

    resetAction_ = toolbar->addAction("Reset");
    resetAction_->setShortcut(QKeySequence(Qt::Key_R));

    toolbar->addSeparator();

    QActionGroup* algorithmGroup = new QActionGroup(this);
    dijkstraAction_ = toolbar->addAction("Dijkstra");
    dijkstraAction_->setCheckable(true);
    dijkstraAction_->setShortcut(QKeySequence(Qt::Key_1));
    aStarAction_ = toolbar->addAction("A*");
    aStarAction_->setCheckable(true);
    aStarAction_->setShortcut(QKeySequence(Qt::Key_2));

    algorithmGroup->addAction(dijkstraAction_);
    algorithmGroup->addAction(aStarAction_);
    dijkstraAction_->setChecked(true);

    connect(playAction_, &QAction::triggered, this, [this](bool) {
        appState_.togglePlay();
        updatePlayAction();
        updateStatusBar();
    });

    connect(stepAction_, &QAction::triggered, this, [this](bool) {
        appState_.stepOnce();
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(resetAction_, &QAction::triggered, this, [this](bool) {
        appState_.resetSearch();
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(dijkstraAction_, &QAction::triggered, this, [this](bool) {
        appState_.setAlgorithm(AppState::AlgorithmKind::Dijkstra);
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(aStarAction_, &QAction::triggered, this, [this](bool) {
        appState_.setAlgorithm(AppState::AlgorithmKind::AStar);
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    timer_ = new QTimer(this);
    timer_->setInterval(30);
    connect(timer_, &QTimer::timeout, this, [this]() {
        appState_.tick();
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });
    timer_->start();

    updatePlayAction();
    updateStatusBar();
}

void MainWindow::updateStatusBar() {
    QString statusText;
    switch (appState_.status()) {
    case pathcore::SearchStatus::NotStarted:
        statusText = "NotStarted";
        break;
    case pathcore::SearchStatus::Running:
        statusText = "Running";
        break;
    case pathcore::SearchStatus::Found:
        statusText = "Found";
        break;
    case pathcore::SearchStatus::NoPath:
        statusText = "NoPath";
        break;
    }

    const QString algorithmText =
        appState_.algorithm() == AppState::AlgorithmKind::Dijkstra ? "Dijkstra" : "A*";

    statusBar()->showMessage(QString("Status: %1 | Algorithm: %2").arg(statusText, algorithmText));
}

void MainWindow::updatePlayAction() {
    if (!playAction_) {
        return;
    }
    playAction_->setText(appState_.playing() ? "Pause" : "Play");
}
