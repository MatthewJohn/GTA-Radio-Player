#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

    // Create instance of window and show
    MainWindow w;
    w.show();

    // Execute application and exit with response code
    return a.exec();
}
