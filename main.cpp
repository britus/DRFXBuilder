#include "drfxtypes.h"
#include "mainwindow.h"
#include "drfxsandbox.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
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
        QString projectFile;
        const QStringList args = arguments();
        if (args.size() > 1) {
            projectFile = args.at(1);
        }

        /* Override commandline style with our fixed GUI style type */
        /* macintosh, Windows, Fusion */
        QString styleName;
        //qDebug() << QStyleFactory::keys();
#if defined(Q_OS_MACOS)
        styleName = "Fusion"; //"macOS";
#elif defined(Q_OS_LINUX)
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

        m_window = new MainWindow(projectFile);
        m_window->show();
        int rc = QApplication::exec();
        delete m_window;

        return rc;
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            const QUrl url = openEvent->url();
            if (url.isLocalFile()) {
                QFileInfo fi(url.toLocalFile());
                qDebug() << "QEvent::FileOpen ->" << fi.absoluteFilePath();
                m_window->setProjectFileName(fi.absoluteFilePath());
            } else if (url.isValid()) {
                // process according to the URL's schema
                qDebug() << "QEvent::FileOpen ->" << url;
            } else {
                qDebug() << "QEvent::FileOpen ->" << url;
            }
        }
        return QApplication::event(event);
    }

private:
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

private:
    MainWindow* m_window;
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

#ifndef Q_OS_MACOS
    /* QPA standard platform theme */
    //setenv("QT_QPA_PLATFORMTHEME", "qt5ct", 0);
#endif

#ifdef _Q_OS_MACOS_disabled_
    QFileInfo fi(argv[0]);
    QString pluginPath = fi.absolutePath().replace("/MacOS", "/PlugIns", Qt::CaseInsensitive);
    setenv("QT_QPA_PLATFORM_PLUGIN_PATH", qPrintable(pluginPath), 0);
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    DRFXApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
    DRFXApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    DRFXApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
    DRFXApplication::setApplicationDisplayName(APP_TITLE);
    DRFXApplication::setApplicationName(APP_TITLE);
#ifdef Q_OS_MACOS
    DRFXApplication::setApplicationVersion(QStringLiteral( //
            "%1.%2").arg(GetBundleVersion(), GetBuildNumber()));
#else
    DRFXApplication::setApplicationVersion(QStringLiteral("1.0.3.8"));
#endif
    DRFXApplication a(argc, argv);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QFile css(":/theme/appstyleqt5.css");
#else
    QFile css(":/theme/appstyle.css");
#endif

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

    return a.exec();
}
