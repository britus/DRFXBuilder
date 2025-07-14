#include "drfxtypes.h"
#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QFileOpenEvent>
#include <QLocale>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QTranslator>

class DRFXApplication : public QApplication
{
public:
    DRFXApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {}

    int exec()
    {
        /* Override commandline style with our fixed GUI style type */
        /* macintosh, Windows, Fusion */
        QString styleName;

#if defined(Q_OS_MACOS) || defined(Q_OS_UNIX)
        styleName = "Fusion";
#else
        styleName = "Windows";
#endif

        /* configure custom GUI style hinter */
        QStyle *style;
        if ((style = QStyleFactory::create(styleName))) {
            ApplicationStyle *myStyle = new ApplicationStyle();
            myStyle->setBaseStyle(style);
            setStyle(myStyle);
        }

        return QApplication::exec();
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

    class ApplicationStyle : public QProxyStyle
    {
    public:
        int styleHint(StyleHint hint,
                      const QStyleOption *option = nullptr, //
                      const QWidget *widget = nullptr,      //
                      QStyleHintReturn *returnData = nullptr) const override
        {
            switch (hint) {
                case QStyle::SH_ComboBox_Popup: {
                    return 0;
                }
                case QStyle::SH_MessageBox_CenterButtons: {
                    return 0;
                }
                case QStyle::SH_FocusFrame_AboveWidget: {
                    return 1;
                }
                default: {
                    break;
                }
            }
            return QProxyStyle::styleHint(hint, option, widget, returnData);
        }
    };
};

int main(int argc, char *argv[])
{
    /* Set specific QT debug message pattern */
    setenv("QT_MESSAGE_PATTERN", "%{time process} %{threadid} %{type} %{category} %{function} %{message}", 0);

    /* Force Bluetooth LE HCI Kernel API */
    setenv("BLUETOOTH_FORCE_DBUS_LE_VERSION", "4.00", 0);

    /* Force Bluetooth LE DBus API */
    // setenv("BLUETOOTH_FORCE_DBUS_LE_VERSION", "5.48", 0);
    // setenv("BLUETOOTH_FORCE_DBUS_LE_VERSION", "5.76", 0);

    /* MacOSX bluetooth LE event dispatching */
    setenv("QT_EVENT_DISPATCHER_CORE_FOUNDATION", "1", 0);
    setenv("BT_TEST_DEVICE", "1", 0);

    /* QPA standard platform theme */
    setenv("QT_QPA_PLATFORMTHEME", "qt5ct", 0);

    DRFXApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    DRFXApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    DRFXApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
    DRFXApplication::setApplicationDisplayName(APP_TITLE);
    DRFXApplication::setApplicationName(APP_TITLE);
    DRFXApplication::setApplicationVersion("1.0.2");
    DRFXApplication a(argc, argv);

#if 1 /*WIP*/
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
