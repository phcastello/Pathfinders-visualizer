#include "VersusView.h"

#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

#include "AppState.h"
#include "GridView.h"

namespace {
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

VersusView::VersusView(QWidget* parent)
    : QWidget(parent) {
    QHBoxLayout* rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(12);

    auto createPanel = [this, rootLayout](QLabel*& headerOut,
                           GridView*& gridOut,
                           QWidget*& controlsOut,
                           QVBoxLayout*& controlsLayoutOut) {
        QFrame* frame = new QFrame(this);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setLineWidth(1);

        QVBoxLayout* panelLayout = new QVBoxLayout(frame);
        panelLayout->setContentsMargins(8, 8, 8, 8);
        panelLayout->setSpacing(6);

        headerOut = new QLabel("...", frame);
        headerOut->setAlignment(Qt::AlignCenter);
        QFont headerFont = headerOut->font();
        headerFont.setBold(true);
        headerOut->setFont(headerFont);

        controlsOut = new QWidget(frame);
        controlsLayoutOut = new QVBoxLayout(controlsOut);
        controlsLayoutOut->setContentsMargins(0, 0, 0, 0);
        controlsLayoutOut->setSpacing(4);
        controlsLayoutOut->setAlignment(Qt::AlignTop);

        gridOut = new GridView(frame);

        panelLayout->addWidget(headerOut);
        panelLayout->addWidget(controlsOut);
        panelLayout->addWidget(gridOut, 1);

        rootLayout->addWidget(frame, 1);
    };

    createPanel(leftHeader_, leftGrid_, leftControls_, leftControlsLayout_);
    createPanel(rightHeader_, rightGrid_, rightControls_, rightControlsLayout_);
}

void VersusView::setStates(AppState* left, AppState* right) {
    leftState_ = left;
    rightState_ = right;
    if (leftGrid_) {
        leftGrid_->setAppState(leftState_);
    }
    if (rightGrid_) {
        rightGrid_->setAppState(rightState_);
    }
    refreshHeaders();
}

GridView* VersusView::leftGrid() const {
    return leftGrid_;
}

GridView* VersusView::rightGrid() const {
    return rightGrid_;
}

QWidget* VersusView::leftControlsWidget() const {
    return leftControls_;
}

QWidget* VersusView::rightControlsWidget() const {
    return rightControls_;
}

QVBoxLayout* VersusView::leftControlsLayout() const {
    return leftControlsLayout_;
}

QVBoxLayout* VersusView::rightControlsLayout() const {
    return rightControlsLayout_;
}

void VersusView::refreshHeaders() {
    auto makeHeader = [](const AppState* state) {
        if (!state) {
            return QString("No State | -- | -- ms");
        }
        const QString algo = algorithmText(state->algorithm());
        const QString status = statusText(state->status());
        const QString time = QString::number(state->algoTimeMs(), 'f', 2);
        const QString weights = state->useWeights() ? "On" : "Off";
        const QString neighbor =
            state->neighborMode() == pathcore::NeighborMode::Eight ? "8" : "4";
        const QString turn =
            state->penalizeTurns() ? QString::number(state->turnPenalty()) : "Off";
        return QString("%1 | %2 | %3 ms | W:%4 | N:%5 | TP:%6")
            .arg(algo)
            .arg(status)
            .arg(time)
            .arg(weights)
            .arg(neighbor)
            .arg(turn);
    };

    if (leftHeader_) {
        leftHeader_->setText(makeHeader(leftState_));
    }
    if (rightHeader_) {
        rightHeader_->setText(makeHeader(rightState_));
    }
}
