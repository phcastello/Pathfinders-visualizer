#pragma once

#include <QDialog>

#include "LaunchOptions.h"

class QPushButton;

class LaunchDialog : public QDialog {
public:
    explicit LaunchDialog(QWidget* parent = nullptr);

    LaunchOptions options() const;

private:
    AppMode selectedMode_{AppMode::Single};
    QPushButton* singleButton_{nullptr};
    QPushButton* versusButton_{nullptr};
};
