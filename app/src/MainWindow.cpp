#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QWidgetAction>

#include "GridView.h"

namespace {
constexpr int kSpeedMin = 1;
constexpr int kSpeedMax = 100;
constexpr int kMaxIntervalMs = 30;
constexpr int kTurnPenaltyMin = 1;
constexpr int kTurnPenaltyMax = 10;

int intervalForSpeed(int speed) {
    if (speed >= kSpeedMax) {
        return 0;
    }
    if (speed < kSpeedMin) {
        speed = kSpeedMin;
    }
    const int steps = kSpeedMax - kSpeedMin;
    const int numerator = kMaxIntervalMs * (kSpeedMax - speed);
    int interval = (numerator + steps - 1) / steps;
    if (interval < 1) {
        interval = 1;
    }
    return interval;
}
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("PathViz");
    resize(900, 600);

    gridView_ = new GridView(this);
    gridView_->setAppState(&appState_);
    setCentralWidget(gridView_);

    QToolBar* toolbar = addToolBar("Controls");
    toolbar->setMovable(false);

    QAction* newAction = toolbar->addAction("New");
    newAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

    QAction* openAction = toolbar->addAction("Open");
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));

    QAction* saveAction = toolbar->addAction("Save");
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

    toolbar->addSeparator();

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

    toolbar->addSeparator();

    weightsAction_ = toolbar->addAction("Weights");
    weightsAction_->setCheckable(true);
    weightsAction_->setShortcut(QKeySequence(Qt::Key_T));
    weightsAction_->setChecked(appState_.useWeights());

    QAction* diagonalAction = toolbar->addAction("Diagonal");
    diagonalAction->setCheckable(true);
    diagonalAction->setShortcut(QKeySequence(Qt::Key_D));
    diagonalAction->setChecked(appState_.neighborMode() == pathcore::NeighborMode::Eight);

    QAction* cornerAction = toolbar->addAction("CornerCut");
    cornerAction->setCheckable(true);
    cornerAction->setShortcut(QKeySequence(Qt::Key_K));
    cornerAction->setChecked(appState_.allowCornerCutting());
    cornerAction->setEnabled(diagonalAction->isChecked());

    turnPenaltyAction_ = toolbar->addAction("TurnPenalty");
    turnPenaltyAction_->setCheckable(true);
    turnPenaltyAction_->setShortcut(QKeySequence(Qt::Key_Z));
    turnPenaltyAction_->setChecked(appState_.penalizeTurns());

    turnPenaltySpin_ = new QSpinBox(toolbar);
    turnPenaltySpin_->setRange(kTurnPenaltyMin, kTurnPenaltyMax);
    turnPenaltySpin_->setValue(appState_.turnPenalty());
    turnPenaltySpin_->setEnabled(turnPenaltyAction_->isChecked());
    QWidgetAction* turnPenaltySpinAction = new QWidgetAction(toolbar);
    turnPenaltySpinAction->setDefaultWidget(turnPenaltySpin_);
    toolbar->addAction(turnPenaltySpinAction);

    toolbar->addSeparator();

    QActionGroup* toolsGroup = new QActionGroup(this);
    toolsGroup->setExclusive(true);

    QAction* wallAction = toolbar->addAction("Wall");
    wallAction->setCheckable(true);
    wallAction->setShortcut(QKeySequence(Qt::Key_W));

    QAction* eraseAction = toolbar->addAction("Erase");
    eraseAction->setCheckable(true);
    eraseAction->setShortcut(QKeySequence(Qt::Key_E));

    QAction* startAction = toolbar->addAction("Start");
    startAction->setCheckable(true);
    startAction->setShortcut(QKeySequence(Qt::Key_S));

    QAction* goalAction = toolbar->addAction("Goal");
    goalAction->setCheckable(true);
    goalAction->setShortcut(QKeySequence(Qt::Key_G));

    QAction* costAction = toolbar->addAction("Cost");
    costAction->setCheckable(true);
    costAction->setShortcut(QKeySequence(Qt::Key_Q));

    toolsGroup->addAction(wallAction);
    toolsGroup->addAction(eraseAction);
    toolsGroup->addAction(startAction);
    toolsGroup->addAction(goalAction);
    toolsGroup->addAction(costAction);
    wallAction->setChecked(true);
    appState_.setTool(AppState::EditTool::DrawWall);

    QSpinBox* costSpin = new QSpinBox(toolbar);
    costSpin->setRange(1, 10);
    costSpin->setValue(appState_.paintCost());
    QWidgetAction* costSpinAction = new QWidgetAction(toolbar);
    costSpinAction->setDefaultWidget(costSpin);
    toolbar->addAction(costSpinAction);

    QLabel* speedLabel = new QLabel("Speed", toolbar);
    toolbar->addWidget(speedLabel);

    QSpinBox* speedSpin = new QSpinBox(toolbar);
    speedSpin->setRange(kSpeedMin, kSpeedMax);
    speedSpin->setValue(appState_.stepsPerTick());
    QWidgetAction* speedSpinAction = new QWidgetAction(toolbar);
    speedSpinAction->setDefaultWidget(speedSpin);
    toolbar->addAction(speedSpinAction);

    toolbar->addSeparator();

    QAction* clearAction = toolbar->addAction("Clear");
    clearAction->setShortcut(QKeySequence(Qt::Key_C));

    connect(newAction, &QAction::triggered, this, [this, costSpin](bool) {
        appState_.newMap();
        costSpin->setValue(appState_.paintCost());
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(openAction, &QAction::triggered, this, [this](bool) {
        const QString path = QFileDialog::getOpenFileName(
            this, "Open Map", QString(), "PathViz Map (*.pvz);;All Files (*)");
        if (path.isEmpty()) {
            return;
        }
        std::string err;
        if (!appState_.loadMap(path.toStdString(), &err)) {
            QMessageBox::warning(this, "Open Map", QString::fromStdString(err));
            return;
        }
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(saveAction, &QAction::triggered, this, [this](bool) {
        QString path = QFileDialog::getSaveFileName(
            this, "Save Map", QString(), "PathViz Map (*.pvz);;All Files (*)");
        if (path.isEmpty()) {
            return;
        }
        const QFileInfo info(path);
        if (info.suffix().isEmpty()) {
            path += ".pvz";
        }
        std::string err;
        if (!appState_.saveMap(path.toStdString(), &err)) {
            QMessageBox::warning(this, "Save Map", QString::fromStdString(err));
            return;
        }
    });

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

    connect(weightsAction_, &QAction::toggled, this, [this](bool checked) {
        appState_.setUseWeights(checked);
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(diagonalAction, &QAction::toggled, this, [this, cornerAction](bool checked) {
        appState_.setNeighborMode(
            checked ? pathcore::NeighborMode::Eight : pathcore::NeighborMode::Four);
        cornerAction->setEnabled(checked);
        if (!checked) {
            cornerAction->setChecked(false);
        }
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(cornerAction, &QAction::toggled, this, [this](bool checked) {
        appState_.setCornerCutting(checked);
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(turnPenaltyAction_, &QAction::toggled, this, [this](bool checked) {
        appState_.setPenalizeTurns(checked);
        if (turnPenaltySpin_) {
            turnPenaltySpin_->setEnabled(checked);
        }
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(turnPenaltySpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        appState_.setTurnPenalty(value);
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    connect(wallAction, &QAction::triggered, this, [this](bool) {
        appState_.setTool(AppState::EditTool::DrawWall);
        updateStatusBar();
    });

    connect(eraseAction, &QAction::triggered, this, [this](bool) {
        appState_.setTool(AppState::EditTool::EraseWall);
        updateStatusBar();
    });

    connect(startAction, &QAction::triggered, this, [this](bool) {
        appState_.setTool(AppState::EditTool::SetStart);
        updateStatusBar();
    });

    connect(goalAction, &QAction::triggered, this, [this](bool) {
        appState_.setTool(AppState::EditTool::SetGoal);
        updateStatusBar();
    });

    connect(costAction, &QAction::triggered, this, [this](bool) {
        appState_.setTool(AppState::EditTool::PaintCost);
        updateStatusBar();
    });

    connect(costSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        appState_.setPaintCost(value);
        updateStatusBar();
    });

    connect(speedSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        appState_.setStepsPerTick(value);
        if (timer_) {
            timer_->setInterval(intervalForSpeed(value));
        }
        updateStatusBar();
    });

    connect(clearAction, &QAction::triggered, this, [this](bool) {
        appState_.clearWalls();
        gridView_->update();
        updatePlayAction();
        updateStatusBar();
    });

    timer_ = new QTimer(this);
    timer_->setInterval(intervalForSpeed(appState_.stepsPerTick()));
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
    if (weightsAction_ && weightsAction_->isChecked() != appState_.useWeights()) {
        QSignalBlocker blocker(weightsAction_);
        weightsAction_->setChecked(appState_.useWeights());
    }
    if (turnPenaltyAction_ && turnPenaltyAction_->isChecked() != appState_.penalizeTurns()) {
        QSignalBlocker blocker(turnPenaltyAction_);
        turnPenaltyAction_->setChecked(appState_.penalizeTurns());
    }
    if (turnPenaltySpin_ && turnPenaltySpin_->value() != appState_.turnPenalty()) {
        QSignalBlocker blocker(turnPenaltySpin_);
        turnPenaltySpin_->setValue(appState_.turnPenalty());
    }
    if (turnPenaltySpin_) {
        turnPenaltySpin_->setEnabled(appState_.penalizeTurns());
    }

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

    QString toolText;
    switch (appState_.tool()) {
    case AppState::EditTool::DrawWall:
        toolText = "Wall";
        break;
    case AppState::EditTool::EraseWall:
        toolText = "Erase";
        break;
    case AppState::EditTool::SetStart:
        toolText = "Start";
        break;
    case AppState::EditTool::SetGoal:
        toolText = "Goal";
        break;
    case AppState::EditTool::PaintCost:
        toolText = QString("Cost(%1)").arg(appState_.paintCost());
        break;
    }

    QString costSegment;
    if (appState_.tool() != AppState::EditTool::PaintCost) {
        costSegment = QString(" | Cost: %1").arg(appState_.paintCost());
    }

    const QString weightsText = appState_.useWeights() ? "On" : "Off";
    const QString neighborText =
        appState_.neighborMode() == pathcore::NeighborMode::Eight ? "8" : "4";
    const QString cornerText = appState_.allowCornerCutting() ? "On" : "Off";
    const QString turnText =
        appState_.penalizeTurns() ? QString::number(appState_.turnPenalty()) : "Off";
    const QString speedText = QString::number(appState_.stepsPerTick());
    const QString timeText = QString::number(appState_.algoTimeMs(), 'f', 2);

    statusBar()->showMessage(
        QString("Status: %1 | Alg: %2 | Tool: %3%4 | Weights: %5 | Neighbors: %6 | Corner: %7 | "
                "TurnPenalty: %8 | Speed: %9 | Time: %10 ms")
            .arg(statusText, algorithmText, toolText, costSegment, weightsText, neighborText,
                cornerText, turnText, speedText, timeText));
}

void MainWindow::updatePlayAction() {
    if (!playAction_) {
        return;
    }
    playAction_->setText(appState_.playing() ? "Pause" : "Play");
}
