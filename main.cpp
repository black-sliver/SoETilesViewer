#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("SoE");
    QCoreApplication::setOrganizationDomain("evermizer.com");
    QCoreApplication::setApplicationName("SoETilesViewer");
    MainWindow w;
    w.show();
    return a.exec();
}
