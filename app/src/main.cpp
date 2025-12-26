#include <QApplication>

#include "LaunchDialog.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    LaunchDialog dialog;
    if (dialog.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window(dialog.options());
    window.show();

    return app.exec();
}
