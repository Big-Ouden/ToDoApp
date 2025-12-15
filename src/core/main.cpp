#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Configuration des informations de l'application
    QCoreApplication::setApplicationName("ToDoApp");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setOrganizationName("ToDoApp");
    
    // Parser les arguments de ligne de commande
    QCommandLineParser parser;
    parser.setApplicationDescription("Application de gestion de tâches");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Option --debug ou -d pour activer les messages de debug
    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   "Active les messages de debug dans la console.");
    parser.addOption(debugOption);
    
    // Option --verbose (mode verbeux - encore plus de détails)
    QCommandLineOption verboseOption("verbose",
                                     "Mode verbeux avec détails supplémentaires.");
    parser.addOption(verboseOption);
    
    parser.process(a);
    
    // Configurer le niveau de debug selon les options
    bool debugEnabled = parser.isSet(debugOption);
    bool verboseEnabled = parser.isSet(verboseOption);
    
    if (verboseEnabled) {
        debugEnabled = true; // verbose implique debug
    }
    
    // Stocker les flags globalement pour y accéder depuis n'importe où
    a.setProperty("debugEnabled", debugEnabled);
    a.setProperty("verboseEnabled", verboseEnabled);
    
    if (debugEnabled) {
        qDebug() << "=== ToDoApp Debug Mode ===";
        qDebug() << "Version:" << QCoreApplication::applicationVersion();
        qDebug() << "Verbose:" << (verboseEnabled ? "Oui" : "Non");
        qDebug() << "Locale:" << QLocale::system().name();
    }

    // Le traducteur est maintenant géré entièrement par MainWindow
    // pour éviter les conflits lors du changement de langue

    MainWindow w;
    w.show();
    return a.exec();
}
