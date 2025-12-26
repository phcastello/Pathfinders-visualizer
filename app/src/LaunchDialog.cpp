#include "LaunchDialog.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>

LaunchDialog::LaunchDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("PathViz Launcher");
    setModal(true);
    resize(900, 600);
    setMinimumSize(900, 600);
    setStyleSheet(
        "QDialog { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #1d2840, stop:1 #111827); }"
        "QLabel#LauncherTitle { color: #f8fafc; }"
        "QLabel#LauncherSubtitle { color: #cbd5f5; }"
        "QPushButton#SingleButton { background: #2563eb; color: #f8fafc; "
        "border-radius: 16px; }"
        "QPushButton#SingleButton:hover { background: #3b82f6; }"
        "QPushButton#SingleButton:pressed { background: #1d4ed8; }"
        "QPushButton#VersusButton { background: #f97316; color: #0b1020; "
        "border-radius: 16px; }"
        "QPushButton#VersusButton:hover { background: #fb923c; }"
        "QPushButton#VersusButton:pressed { background: #ea580c; }"
        "QPushButton#ExitButton { color: #e2e8f0; background: rgba(248,250,252,0.08); "
        "border: 1px solid #475569; border-radius: 12px; padding: 8px 26px; "
        "font-weight: 600; }"
        "QPushButton#ExitButton:hover { background: rgba(248,250,252,0.16); "
        "border-color: #64748b; }"
        "QPushButton#ExitButton:pressed { background: rgba(248,250,252,0.22); }");

    QLabel* titleLabel = new QLabel("PathViz", this);
    titleLabel->setObjectName("LauncherTitle");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* subtitleLabel = new QLabel("Choose a mode to get started.", this);
    subtitleLabel->setObjectName("LauncherSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);

    singleButton_ = new QPushButton("Single\nClassic pathfinding", this);
    singleButton_->setObjectName("SingleButton");
    singleButton_->setMinimumSize(260, 180);
    singleButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QFont singleFont = singleButton_->font();
    singleFont.setPointSize(singleFont.pointSize() + 6);
    singleFont.setBold(true);
    singleButton_->setFont(singleFont);

    versusButton_ = new QPushButton("Versus\nHead-to-head (WIP)", this);
    versusButton_->setObjectName("VersusButton");
    versusButton_->setMinimumSize(260, 180);
    versusButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QFont versusFont = versusButton_->font();
    versusFont.setPointSize(versusFont.pointSize() + 6);
    versusFont.setBold(true);
    versusButton_->setFont(versusFont);

    QDialogButtonBox* buttons =
        new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    QPushButton* exitButton = buttons->button(QDialogButtonBox::Cancel);
    if (exitButton) {
        exitButton->setText("Exit");
        exitButton->setObjectName("ExitButton");
        exitButton->setMinimumWidth(160);
    }

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(singleButton_, &QPushButton::clicked, this, [this]() {
        selectedMode_ = AppMode::Single;
        accept();
    });
    connect(versusButton_, &QPushButton::clicked, this, [this]() {
        selectedMode_ = AppMode::Versus;
        accept();
    });

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(32);
    contentLayout->addStretch();
    contentLayout->addWidget(singleButton_, 1);
    contentLayout->addWidget(versusButton_, 1);
    contentLayout->addStretch();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(18);
    layout->addWidget(titleLabel);
    layout->addWidget(subtitleLabel);
    layout->addSpacing(8);
    layout->addStretch(1);
    layout->addLayout(contentLayout, 2);
    layout->addStretch(1);
    layout->addWidget(buttons, 0, Qt::AlignHCenter);
}

LaunchOptions LaunchDialog::options() const {
    LaunchOptions opts;
    opts.mode = selectedMode_;
    return opts;
}
