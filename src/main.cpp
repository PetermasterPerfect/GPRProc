#include "mainwindow.h"
#include <complex>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
    MainWindow window(argc >= 2 ? argv[1] : nullptr);
    window.show();
    return app.exec();
}
