#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Le traducteur est maintenant géré entièrement par MainWindow
    // pour éviter les conflits lors du changement de langue

    MainWindow w;
    w.show();
    return a.exec();
}
