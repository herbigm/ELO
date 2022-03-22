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
    app.setWindowIcon(QIcon(":/icons/Icon.ico"));

    // settings for the translations
    QString localeString = QLocale::system().name();
    QLocale locale = QLocale(localeString);
    QLocale::setDefault(locale);
    QTranslator translator;
    if (translator.load(QCoreApplication::applicationDirPath() + QDir::separator() + QString("ELO-NG_%1.qm").arg(localeString))) {
        app.installTranslator(&translator);
    }

    // starting the MainWindow
    ELOMainWindow w;
    w.showMaximized();
    return app.exec();
}
