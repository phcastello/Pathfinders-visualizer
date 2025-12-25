#pragma once

#include <QWidget>

class AppState;

class GridView : public QWidget {
public:
    explicit GridView(QWidget* parent = nullptr);

    void setAppState(AppState* state);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    AppState* state_{nullptr};
};
