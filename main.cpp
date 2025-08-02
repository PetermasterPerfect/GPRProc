#include "mainwindow.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
    MainWindow window(argv[1]);
    window.show();
    return app.exec();
}
