#include "elomainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QWebEngineSettings>
#include <QWebEngineProfile>

int main(int argc, char *argv[])
{
    // general settings
    QCoreApplication::setOrganizationName("TU Bergakademie Freiberg");
    QCoreApplication::setApplicationName("ELO");
    QCoreApplication::setApplicationVersion("1.49");

    QApplication app(argc, argv);

    // settings for the translations
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ELO-NG_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // starting the MainWindow
    ELOMainWindow w;
    w.showMaximized();
    return app.exec();
}
