#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication::setApplicationDisplayName("EoF DRFX Builder");
    QApplication::setApplicationName("EoF DRFX Builder");
    QApplication::setApplicationVersion("1.0.2");
    QApplication a(argc, argv);

    QFile css(":/theme/appstyle.css");
    if (css.open(QFile::ReadOnly)) {
        a.setStyleSheet(css.readAll());
        css.close();
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "DRFXBuilder_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}
