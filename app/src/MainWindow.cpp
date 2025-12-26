#include "MainWindow.h"

#include <algorithm>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QToolButton>
#include <QToolBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "GridView.h"
#include "VersusView.h"

namespace {
constexpr int kSpeedMin = 1;
constexpr int kSpeedMax = 100;
constexpr int kMaxIntervalMs = 30;
constexpr int kTurnPenaltyMin = 1;
constexpr int kTurnPenaltyMax = 10;
constexpr int kGridSizeMin = 5;
constexpr int kGridSizeMax = 200;
constexpr int kToggleMinWidth = 72;
constexpr int kSpinBoxWidth = 64;

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

AppState::AlgorithmKind toAppAlgorithm(AlgoKind kind) {
    return kind == AlgoKind::AStar ? AppState::AlgorithmKind::AStar
                                   : AppState::AlgorithmKind::Dijkstra;
}

QString statusText(pathcore::SearchStatus status) {
    switch (status) {
    case pathcore::SearchStatus::NotStarted:
        return "NotStarted";
    case pathcore::SearchStatus::Running:
        return "Running";
    case pathcore::SearchStatus::Found:
        return "Found";
    case pathcore::SearchStatus::NoPath:
        return "NoPath";
    }
    return "Unknown";
}

QString algorithmText(AppState::AlgorithmKind kind) {
    return kind == AppState::AlgorithmKind::AStar ? "A*" : "Dijkstra";
}
} // namespace

MainWindow::MainWindow(const LaunchOptions& opts, QWidget* parent)
    : QMainWindow(parent)
    , options_(opts) {
    setWindowTitle("PathViz");
    resize(900, 600);
    setStyleSheet(
        "QMainWindow { background: #f3f6fb; }"
        "QToolBar { background: #f8fafc; border-bottom: 1px solid #d6dee8; spacing: 6px; }"
        "QToolButton { background: #e2e8f0; color: #1e293b; border: 1px solid #cbd5e1; "
        "border-radius: 6px; padding: 4px 10px; }"
        "QToolButton:hover { background: #dbeafe; border-color: #bfdbfe; }"
        "QToolButton:checked { background: #2563eb; color: #f8fafc; border-color: #1d4ed8; }"
        "QToolButton:disabled { color: #94a3b8; background: #e2e8f0; border-color: #cbd5e1; }"
        "QSpinBox QLineEdit { background: #ffffff; border: 1px solid #cbd5e1; "
        "border-radius: 6px; padding: 2px 6px; }"
        "QStatusBar { background: #0f172a; color: #e2e8f0; }"
        "QStatusBar::item { border: none; }");

    if (options_.mode == AppMode::Single) {
        appState_.setAlgorithm(toAppAlgorithm(options_.singleAlgo));
    } else {
        leftState_.setAlgorithm(toAppAlgorithm(options_.leftAlgo));
        rightState_.setAlgorithm(toAppAlgorithm(options_.rightAlgo));
    }

    QStackedWidget* centralStack = new QStackedWidget(this);
    gridView_ = new GridView(centralStack);
    gridView_->setAppState(&appState_);

    versusView_ = new VersusView(centralStack);
    versusView_->setStates(&leftState_, &rightState_);
    versusView_->leftGrid()->setInteractive(true);
    versusView_->rightGrid()->setInteractive(false);
    versusView_->leftGrid()->setEditedCallback([this]() { syncRightFromLeft(); });

    centralStack->addWidget(gridView_);
    centralStack->addWidget(versusView_);
    centralStack->setCurrentWidget(isVersus() ? static_cast<QWidget*>(versusView_)
                                              : static_cast<QWidget*>(gridView_));
    setCentralWidget(centralStack);

    if (isVersus()) {
        syncRightFromLeft();
    }

    AppState& controlState = isVersus() ? leftState_ : appState_;

    QToolBar* toolbar = addToolBar("Controls");
    toolbar->setMovable(false);

    QAction* newAction = toolbar->addAction("New");
    newAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

    QAction* openAction = toolbar->addAction("Open");
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));

    QAction* saveAction = toolbar->addAction("Save");
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

    QAction* gridAction = toolbar->addAction("Grid");
    gridAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));

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
    if (controlState.algorithm() == AppState::AlgorithmKind::Dijkstra) {
        dijkstraAction_->setChecked(true);
    } else {
        aStarAction_->setChecked(true);
    }

    if (isVersus()) {
        dijkstraAction_->setVisible(false);
        dijkstraAction_->setEnabled(false);
        aStarAction_->setVisible(false);
        aStarAction_->setEnabled(false);
    }

    toolbar->addSeparator();

    QAction* diagonalAction = nullptr;
    QAction* cornerAction = nullptr;
    QSpinBox* speedSpin = nullptr;

    if (!isVersus()) {
        weightsAction_ = toolbar->addAction("Weights");
        weightsAction_->setCheckable(true);
        weightsAction_->setShortcut(QKeySequence(Qt::Key_T));
        weightsAction_->setChecked(controlState.useWeights());

        diagonalAction = toolbar->addAction("Diagonal");
        diagonalAction->setCheckable(true);
        diagonalAction->setShortcut(QKeySequence(Qt::Key_D));
        diagonalAction->setChecked(controlState.neighborMode() == pathcore::NeighborMode::Eight);

        cornerAction = toolbar->addAction("CornerCut");
        cornerAction->setCheckable(true);
        cornerAction->setShortcut(QKeySequence(Qt::Key_K));
        cornerAction->setChecked(controlState.allowCornerCutting());
        cornerAction->setEnabled(diagonalAction->isChecked());

        turnPenaltyAction_ = toolbar->addAction("TurnPenalty");
        turnPenaltyAction_->setCheckable(true);
        turnPenaltyAction_->setShortcut(QKeySequence(Qt::Key_Z));
        turnPenaltyAction_->setChecked(controlState.penalizeTurns());

        turnPenaltySpin_ = new QSpinBox(toolbar);
        turnPenaltySpin_->setRange(kTurnPenaltyMin, kTurnPenaltyMax);
        turnPenaltySpin_->setValue(controlState.turnPenalty());
        turnPenaltySpin_->setEnabled(turnPenaltyAction_->isChecked());
        QWidgetAction* turnPenaltySpinAction = new QWidgetAction(toolbar);
        turnPenaltySpinAction->setDefaultWidget(turnPenaltySpin_);
        toolbar->addAction(turnPenaltySpinAction);
        toolbar->addSeparator();
    }

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
    controlState.setTool(AppState::EditTool::DrawWall);

    QSpinBox* costSpin = new QSpinBox(toolbar);
    costSpin->setRange(1, 10);
    costSpin->setValue(controlState.paintCost());
    QWidgetAction* costSpinAction = new QWidgetAction(toolbar);
    costSpinAction->setDefaultWidget(costSpin);
    toolbar->addAction(costSpinAction);

    if (!isVersus()) {
        QLabel* speedLabel = new QLabel("Speed", toolbar);
        toolbar->addWidget(speedLabel);

        speedSpin = new QSpinBox(toolbar);
        speedSpin->setRange(kSpeedMin, kSpeedMax);
        speedSpin->setValue(controlState.stepsPerTick());
        QWidgetAction* speedSpinAction = new QWidgetAction(toolbar);
        speedSpinAction->setDefaultWidget(speedSpin);
        toolbar->addAction(speedSpinAction);
    }

    toolbar->addSeparator();

    QAction* clearAction = toolbar->addAction("Clear");
    clearAction->setShortcut(QKeySequence(Qt::Key_C));

    if (isVersus() && versusView_) {
        auto makeToggle =
            [this](const QString& text, bool checked, QAction*& action, QWidget* parent) {
                QWidget* targetParent = parent ? parent : this;
                action = new QAction(text, targetParent);
                action->setCheckable(true);
                action->setChecked(checked);
                QToolButton* button = new QToolButton(targetParent);
                button->setDefaultAction(action);
                return button;
            };

        auto buildSideControls = [this, &makeToggle](QVBoxLayout* layout,
                                    QWidget* parent,
                                    AppState& state,
                                    QAction*& weightsAction,
                                    QAction*& diagonalAction,
                                    QAction*& cornerAction,
                                    QAction*& turnAction,
                                    QSpinBox*& turnSpin,
                                    QSpinBox*& speedSpin,
                                    QComboBox*& algoCombo) {
            if (!layout) {
                return;
            }
            QWidget* targetParent = parent ? parent : this;

            QGridLayout* grid = new QGridLayout();
            grid->setContentsMargins(0, 0, 0, 0);
            grid->setHorizontalSpacing(6);
            grid->setVerticalSpacing(4);

            QLabel* algoLabel = new QLabel("Alg", targetParent);
            algoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            algoCombo = new QComboBox(targetParent);
            algoCombo->addItems({"Dijkstra", "A*"});
            algoCombo->setCurrentIndex(
                state.algorithm() == AppState::AlgorithmKind::AStar ? 1 : 0);
            algoCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            algoCombo->setMinimumContentsLength(8);
            algoCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            QToolButton* weightsButton =
                makeToggle("Weights", state.useWeights(), weightsAction, targetParent);
            weightsButton->setMinimumWidth(kToggleMinWidth);
            weightsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            QToolButton* diagonalButton = makeToggle("Diag",
                state.neighborMode() == pathcore::NeighborMode::Eight,
                diagonalAction,
                targetParent);
            diagonalButton->setMinimumWidth(kToggleMinWidth);
            diagonalButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            QToolButton* cornerButton = makeToggle(
                "Corner", state.allowCornerCutting(), cornerAction, targetParent);
            cornerButton->setMinimumWidth(kToggleMinWidth);
            cornerButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            cornerButton->setEnabled(diagonalAction->isChecked());

            QToolButton* turnButton =
                makeToggle("Turn", state.penalizeTurns(), turnAction, targetParent);
            turnButton->setMinimumWidth(kToggleMinWidth);
            turnButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            QLabel* turnValLabel = new QLabel("Val", targetParent);
            turnValLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            turnSpin = new QSpinBox(targetParent);
            turnSpin->setRange(kTurnPenaltyMin, kTurnPenaltyMax);
            turnSpin->setValue(state.turnPenalty());
            turnSpin->setEnabled(turnAction->isChecked());
            turnSpin->setFixedWidth(kSpinBoxWidth);

            QLabel* speedLabel = new QLabel("Speed", targetParent);
            speedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            speedSpin = new QSpinBox(targetParent);
            speedSpin->setRange(kSpeedMin, kSpeedMax);
            speedSpin->setValue(state.stepsPerTick());
            speedSpin->setFixedWidth(kSpinBoxWidth);

            grid->addWidget(algoLabel, 0, 0);
            grid->addWidget(algoCombo, 0, 1);
            grid->addWidget(weightsButton, 0, 2);
            grid->addWidget(diagonalButton, 0, 3);
            grid->addWidget(cornerButton, 0, 4);

            grid->addWidget(turnButton, 1, 0);
            grid->addWidget(turnValLabel, 1, 1);
            grid->addWidget(turnSpin, 1, 2);
            grid->addWidget(speedLabel, 1, 3);
            grid->addWidget(speedSpin, 1, 4);
            grid->setColumnStretch(5, 1);

            layout->addLayout(grid);
        };

        QComboBox* leftAlgoCombo = nullptr;
        QComboBox* rightAlgoCombo = nullptr;

        buildSideControls(versusView_->leftControlsLayout(),
            versusView_->leftControlsWidget(),
            leftState_,
            leftWeightsAction_,
            leftDiagonalAction_,
            leftCornerAction_,
            leftTurnPenaltyAction_,
            leftTurnPenaltySpin_,
            leftSpeedSpin_,
            leftAlgoCombo);

        buildSideControls(versusView_->rightControlsLayout(),
            versusView_->rightControlsWidget(),
            rightState_,
            rightWeightsAction_,
            rightDiagonalAction_,
            rightCornerAction_,
            rightTurnPenaltyAction_,
            rightTurnPenaltySpin_,
            rightSpeedSpin_,
            rightAlgoCombo);

        if (leftAlgoCombo) {
            connect(leftAlgoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int index) {
                    leftState_.setAlgorithm(index == 1 ? AppState::AlgorithmKind::AStar
                                                       : AppState::AlgorithmKind::Dijkstra);
                    updateViewsCurrentMode();
                    updatePlayAction();
                    updateStatusBarCurrentMode();
                });
        }

        if (rightAlgoCombo) {
            connect(rightAlgoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int index) {
                    rightState_.setAlgorithm(index == 1 ? AppState::AlgorithmKind::AStar
                                                        : AppState::AlgorithmKind::Dijkstra);
                    updateViewsCurrentMode();
                    updatePlayAction();
                    updateStatusBarCurrentMode();
                });
        }
    }

    connect(newAction, &QAction::triggered, this, [this, costSpin](bool) {
        if (isVersus()) {
            leftState_.newMap();
            costSpin->setValue(leftState_.paintCost());
            syncRightFromLeft();
        } else {
            appState_.newMap();
            costSpin->setValue(appState_.paintCost());
        }
        updateViewsCurrentMode();
        updatePlayAction();
        updateStatusBarCurrentMode();
    });

    connect(gridAction, &QAction::triggered, this, [this, costSpin](bool) {
        if (!showResizeGridDialog()) {
            return;
        }
        if (isVersus()) {
            costSpin->setValue(leftState_.paintCost());
            syncRightFromLeft();
        } else {
            costSpin->setValue(appState_.paintCost());
        }
        updateViewsCurrentMode();
        updatePlayAction();
        updateStatusBarCurrentMode();
    });

    connect(openAction, &QAction::triggered, this, [this](bool) {
        const QString path = QFileDialog::getOpenFileName(
            this, "Open Map", QString(), "PathViz Map (*.pvz);;All Files (*)");
        if (path.isEmpty()) {
            return;
        }
        std::string err;
        if (isVersus()) {
            if (!leftState_.loadMap(path.toStdString(), &err)) {
                QMessageBox::warning(this, "Open Map", QString::fromStdString(err));
                return;
            }
            syncRightFromLeft();
        } else {
            if (!appState_.loadMap(path.toStdString(), &err)) {
                QMessageBox::warning(this, "Open Map", QString::fromStdString(err));
                return;
            }
        }
        updateViewsCurrentMode();
        updatePlayAction();
        updateStatusBarCurrentMode();
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
        if (isVersus()) {
            if (!leftState_.saveMap(path.toStdString(), &err)) {
                QMessageBox::warning(this, "Save Map", QString::fromStdString(err));
                return;
            }
            return;
        }
        if (!appState_.saveMap(path.toStdString(), &err)) {
            QMessageBox::warning(this, "Save Map", QString::fromStdString(err));
            return;
        }
    });

    connect(playAction_, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            const bool anyPlaying = leftState_.playing() || rightState_.playing();
            if (anyPlaying) {
                leftState_.pause();
                rightState_.pause();
            } else {
                leftState_.togglePlay();
                rightState_.togglePlay();
            }
        } else {
            appState_.togglePlay();
        }
        updatePlayAction();
        updateStatusBarCurrentMode();
    });

    connect(stepAction_, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.stepOnce();
            rightState_.stepOnce();
        } else {
            appState_.stepOnce();
        }
        updateViewsCurrentMode();
        updatePlayAction();
        updateStatusBarCurrentMode();
    });

    connect(resetAction_, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.resetSearch();
            syncRightFromLeft();
        } else {
            appState_.resetSearch();
        }
        updateViewsCurrentMode();
        updatePlayAction();
        updateStatusBarCurrentMode();
    });

    if (!isVersus()) {
        connect(dijkstraAction_, &QAction::triggered, this, [this](bool) {
            appState_.setAlgorithm(AppState::AlgorithmKind::Dijkstra);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(aStarAction_, &QAction::triggered, this, [this](bool) {
            appState_.setAlgorithm(AppState::AlgorithmKind::AStar);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });
    }

    auto updateTimerInterval = [this]() {
        if (!timer_) {
            return;
        }
        const int speed = isVersus()
            ? std::max(leftState_.stepsPerTick(), rightState_.stepsPerTick())
            : appState_.stepsPerTick();
        timer_->setInterval(intervalForSpeed(speed));
    };

    if (isVersus()) {
        connect(leftWeightsAction_, &QAction::toggled, this, [this](bool checked) {
            leftState_.setUseWeights(checked);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(rightWeightsAction_, &QAction::toggled, this, [this](bool checked) {
            rightState_.setUseWeights(checked);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(leftDiagonalAction_, &QAction::toggled, this, [this](bool checked) {
            leftState_.setNeighborMode(
                checked ? pathcore::NeighborMode::Eight : pathcore::NeighborMode::Four);
            if (leftCornerAction_) {
                leftCornerAction_->setEnabled(checked);
                if (!checked) {
                    leftCornerAction_->setChecked(false);
                }
            }
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(rightDiagonalAction_, &QAction::toggled, this, [this](bool checked) {
            rightState_.setNeighborMode(
                checked ? pathcore::NeighborMode::Eight : pathcore::NeighborMode::Four);
            if (rightCornerAction_) {
                rightCornerAction_->setEnabled(checked);
                if (!checked) {
                    rightCornerAction_->setChecked(false);
                }
            }
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(leftCornerAction_, &QAction::toggled, this, [this](bool checked) {
            leftState_.setCornerCutting(checked);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(rightCornerAction_, &QAction::toggled, this, [this](bool checked) {
            rightState_.setCornerCutting(checked);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(leftTurnPenaltyAction_, &QAction::toggled, this, [this](bool checked) {
            leftState_.setPenalizeTurns(checked);
            if (leftTurnPenaltySpin_) {
                leftTurnPenaltySpin_->setEnabled(checked);
            }
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(rightTurnPenaltyAction_, &QAction::toggled, this, [this](bool checked) {
            rightState_.setPenalizeTurns(checked);
            if (rightTurnPenaltySpin_) {
                rightTurnPenaltySpin_->setEnabled(checked);
            }
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(leftTurnPenaltySpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) {
                leftState_.setTurnPenalty(value);
                updateViewsCurrentMode();
                updatePlayAction();
                updateStatusBarCurrentMode();
            });

        connect(rightTurnPenaltySpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) {
                rightState_.setTurnPenalty(value);
                updateViewsCurrentMode();
                updatePlayAction();
                updateStatusBarCurrentMode();
            });

        connect(leftSpeedSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this, updateTimerInterval](int value) {
                leftState_.setStepsPerTick(value);
                updateTimerInterval();
                updateViewsCurrentMode();
                updatePlayAction();
                updateStatusBarCurrentMode();
            });

        connect(rightSpeedSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this, updateTimerInterval](int value) {
                rightState_.setStepsPerTick(value);
                updateTimerInterval();
                updateViewsCurrentMode();
                updatePlayAction();
                updateStatusBarCurrentMode();
            });
    } else {
        connect(weightsAction_, &QAction::toggled, this, [this](bool checked) {
            appState_.setUseWeights(checked);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(diagonalAction, &QAction::toggled, this, [this, cornerAction](bool checked) {
            appState_.setNeighborMode(
                checked ? pathcore::NeighborMode::Eight : pathcore::NeighborMode::Four);
            cornerAction->setEnabled(checked);
            if (!checked) {
                cornerAction->setChecked(false);
            }
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(cornerAction, &QAction::toggled, this, [this](bool checked) {
            appState_.setCornerCutting(checked);
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(turnPenaltyAction_, &QAction::toggled, this, [this](bool checked) {
            appState_.setPenalizeTurns(checked);
            if (turnPenaltySpin_) {
                turnPenaltySpin_->setEnabled(checked);
            }
            updateViewsCurrentMode();
            updatePlayAction();
            updateStatusBarCurrentMode();
        });

        connect(turnPenaltySpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) {
                appState_.setTurnPenalty(value);
                updateViewsCurrentMode();
                updatePlayAction();
                updateStatusBarCurrentMode();
            });

        connect(speedSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this, updateTimerInterval](int value) {
                appState_.setStepsPerTick(value);
                updateTimerInterval();
                updateStatusBarCurrentMode();
            });
    }

    connect(wallAction, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.setTool(AppState::EditTool::DrawWall);
        } else {
            appState_.setTool(AppState::EditTool::DrawWall);
        }
        updateStatusBarCurrentMode();
    });

    connect(eraseAction, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.setTool(AppState::EditTool::EraseWall);
        } else {
            appState_.setTool(AppState::EditTool::EraseWall);
        }
        updateStatusBarCurrentMode();
    });

    connect(startAction, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.setTool(AppState::EditTool::SetStart);
        } else {
            appState_.setTool(AppState::EditTool::SetStart);
        }
        updateStatusBarCurrentMode();
    });

    connect(goalAction, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.setTool(AppState::EditTool::SetGoal);
        } else {
            appState_.setTool(AppState::EditTool::SetGoal);
        }
        updateStatusBarCurrentMode();
    });

    connect(costAction, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.setTool(AppState::EditTool::PaintCost);
        } else {
            appState_.setTool(AppState::EditTool::PaintCost);
        }
        updateStatusBarCurrentMode();
    });

    connect(costSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (isVersus()) {
            leftState_.setPaintCost(value);
        } else {
            appState_.setPaintCost(value);
        }
        updateStatusBarCurrentMode();
    });

    connect(clearAction, &QAction::triggered, this, [this](bool) {
        if (isVersus()) {
            leftState_.clearWalls();
            syncRightFromLeft();
        } else {
            appState_.clearWalls();
        }
        updateViewsCurrentMode();
        updatePlayAction();
        updateStatusBarCurrentMode();
    });

    timer_ = new QTimer(this);
    updateTimerInterval();
    connect(timer_, &QTimer::timeout, this, [this]() {
        tickCurrentMode();
    });
    timer_->start();

    updatePlayAction();
    updateStatusBarCurrentMode();
}

bool MainWindow::isVersus() const {
    return options_.mode == AppMode::Versus;
}

void MainWindow::syncRightFromLeft() {
    const bool keepRightWeights = rightState_.useWeights();
    rightState_.replaceWorld(
        leftState_.grid(), leftState_.start(), leftState_.goal(), keepRightWeights);
    if (versusView_) {
        versusView_->rightGrid()->update();
        versusView_->refreshHeaders();
    }
}

void MainWindow::tickCurrentMode() {
    if (isVersus()) {
        leftState_.tick();
        rightState_.tick();
    } else {
        appState_.tick();
    }
    updateViewsCurrentMode();
    updatePlayAction();
    updateStatusBarCurrentMode();
}

void MainWindow::updateViewsCurrentMode() {
    if (isVersus()) {
        if (versusView_) {
            versusView_->leftGrid()->update();
            versusView_->rightGrid()->update();
        }
    } else {
        if (gridView_) {
            gridView_->update();
        }
    }
}

void MainWindow::updateStatusBarCurrentMode() {
    if (isVersus()) {
        updateStatusBarVersus();
        if (versusView_) {
            versusView_->refreshHeaders();
        }
        return;
    }
    updateStatusBar();
}

bool MainWindow::showResizeGridDialog() {
    AppState& state = isVersus() ? leftState_ : appState_;

    QDialog dialog(this);
    dialog.setWindowTitle("Resize Grid");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLabel* infoLabel = new QLabel(
        "Resizing creates a new empty map (clears walls and costs).", &dialog);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    QFormLayout* formLayout = new QFormLayout();
    QSpinBox* widthSpin = new QSpinBox(&dialog);
    widthSpin->setRange(kGridSizeMin, kGridSizeMax);
    widthSpin->setValue(state.gridWidth());
    formLayout->addRow("Width", widthSpin);

    QSpinBox* heightSpin = new QSpinBox(&dialog);
    heightSpin->setRange(kGridSizeMin, kGridSizeMax);
    heightSpin->setValue(state.gridHeight());
    formLayout->addRow("Height", heightSpin);
    layout->addLayout(formLayout);

    QDialogButtonBox* buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    return state.resizeGrid(widthSpin->value(), heightSpin->value());
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

    const QString statusLabel = statusText(appState_.status());
    const QString algorithmLabel = algorithmText(appState_.algorithm());

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
    const QString gridText =
        QString("%1x%2").arg(appState_.gridWidth()).arg(appState_.gridHeight());

    const QString message =
        QString("Status: %1 | Alg: %2 | Tool: %3%4 | Grid: %5 | Weights: %6 | Neighbors: %7 | "
                "Corner: %8 | TurnPenalty: %9 | Speed: %10 | Time: %11 ms")
            .arg(statusLabel)
            .arg(algorithmLabel)
            .arg(toolText)
            .arg(costSegment)
            .arg(gridText)
            .arg(weightsText)
            .arg(neighborText)
            .arg(cornerText)
            .arg(turnText)
            .arg(speedText)
            .arg(timeText);

    statusBar()->showMessage(message);
}

void MainWindow::updateStatusBarVersus() {
    auto syncSideControls = [](const AppState& state,
                                QAction* weightsAction,
                                QAction* diagonalAction,
                                QAction* cornerAction,
                                QAction* turnAction,
                                QSpinBox* turnSpin,
                                QSpinBox* speedSpin) {
        if (weightsAction && weightsAction->isChecked() != state.useWeights()) {
            QSignalBlocker blocker(weightsAction);
            weightsAction->setChecked(state.useWeights());
        }
        if (diagonalAction) {
            const bool diag = state.neighborMode() == pathcore::NeighborMode::Eight;
            if (diagonalAction->isChecked() != diag) {
                QSignalBlocker blocker(diagonalAction);
                diagonalAction->setChecked(diag);
            }
        }
        if (cornerAction) {
            const bool corner = state.allowCornerCutting();
            if (cornerAction->isChecked() != corner) {
                QSignalBlocker blocker(cornerAction);
                cornerAction->setChecked(corner);
            }
            cornerAction->setEnabled(state.neighborMode() == pathcore::NeighborMode::Eight);
        }
        if (turnAction && turnAction->isChecked() != state.penalizeTurns()) {
            QSignalBlocker blocker(turnAction);
            turnAction->setChecked(state.penalizeTurns());
        }
        if (turnSpin && turnSpin->value() != state.turnPenalty()) {
            QSignalBlocker blocker(turnSpin);
            turnSpin->setValue(state.turnPenalty());
        }
        if (turnSpin) {
            turnSpin->setEnabled(state.penalizeTurns());
        }
        if (speedSpin && speedSpin->value() != state.stepsPerTick()) {
            QSignalBlocker blocker(speedSpin);
            speedSpin->setValue(state.stepsPerTick());
        }
    };

    syncSideControls(leftState_,
        leftWeightsAction_,
        leftDiagonalAction_,
        leftCornerAction_,
        leftTurnPenaltyAction_,
        leftTurnPenaltySpin_,
        leftSpeedSpin_);
    syncSideControls(rightState_,
        rightWeightsAction_,
        rightDiagonalAction_,
        rightCornerAction_,
        rightTurnPenaltyAction_,
        rightTurnPenaltySpin_,
        rightSpeedSpin_);

    const QString leftStatus = statusText(leftState_.status());
    const QString rightStatus = statusText(rightState_.status());
    const QString leftAlg = algorithmText(leftState_.algorithm());
    const QString rightAlg = algorithmText(rightState_.algorithm());
    const QString leftTime = QString::number(leftState_.algoTimeMs(), 'f', 2);
    const QString rightTime = QString::number(rightState_.algoTimeMs(), 'f', 2);
    const QString leftWeights = leftState_.useWeights() ? "On" : "Off";
    const QString rightWeights = rightState_.useWeights() ? "On" : "Off";
    const QString leftNeighbor =
        leftState_.neighborMode() == pathcore::NeighborMode::Eight ? "8" : "4";
    const QString rightNeighbor =
        rightState_.neighborMode() == pathcore::NeighborMode::Eight ? "8" : "4";
    const QString leftCorner = leftState_.allowCornerCutting() ? "On" : "Off";
    const QString rightCorner = rightState_.allowCornerCutting() ? "On" : "Off";
    const QString leftTurn =
        leftState_.penalizeTurns() ? QString::number(leftState_.turnPenalty()) : "Off";
    const QString rightTurn =
        rightState_.penalizeTurns() ? QString::number(rightState_.turnPenalty()) : "Off";
    const QString leftSpeed = QString::number(leftState_.stepsPerTick());
    const QString rightSpeed = QString::number(rightState_.stepsPerTick());

    const QString message =
        QString("L: %1 %2 %3 ms W:%4 N:%5 C:%6 TP:%7 S:%8 | "
                "R: %9 %10 %11 ms W:%12 N:%13 C:%14 TP:%15 S:%16")
            .arg(leftAlg)
            .arg(leftStatus)
            .arg(leftTime)
            .arg(leftWeights)
            .arg(leftNeighbor)
            .arg(leftCorner)
            .arg(leftTurn)
            .arg(leftSpeed)
            .arg(rightAlg)
            .arg(rightStatus)
            .arg(rightTime)
            .arg(rightWeights)
            .arg(rightNeighbor)
            .arg(rightCorner)
            .arg(rightTurn)
            .arg(rightSpeed);

    statusBar()->showMessage(message);
}

void MainWindow::updatePlayAction() {
    if (!playAction_) {
        return;
    }
    const bool playing = isVersus() ? (leftState_.playing() || rightState_.playing())
                                    : appState_.playing();
    playAction_->setText(playing ? "Pause" : "Play");
}
