#include "drfxtypes.h"
#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QLocale>
#include <QTranslator>
#include <QFileOpenEvent>

class DRFXApplication : public QApplication
{
public:
    DRFXApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            const QUrl url = openEvent->url();
            if (url.isLocalFile()) {
                QFile localFile(url.toLocalFile());
                // read from local file
            } else if (url.isValid()) {
                // process according to the URL's schema
            } else {
                // parse openEvent->file()
            }
        }

        return QApplication::event(event);
    }
};

int main(int argc, char *argv[])
{
    DRFXApplication::setApplicationDisplayName(APP_TITLE);
    DRFXApplication::setApplicationName(APP_TITLE);
    DRFXApplication::setApplicationVersion("1.0.2");
    DRFXApplication a(argc, argv);
#if 0 /*WIP*/
    QFile css(":/theme/appstyle.css");
    if (css.open(QFile::ReadOnly)) {
        a.setStyleSheet(css.readAll());
        css.close();
    }
#endif
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
