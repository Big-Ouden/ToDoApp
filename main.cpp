#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Charger traduction système (si disponible)
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ToDoApp_" + QLocale(locale).name();
        // recherche dans ressources et dans répertoire ./i18n
        if (translator.load(":/i18n/" + baseName) ||
            translator.load(QDir::currentPath() + "/i18n/" + baseName + ".qm")) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
