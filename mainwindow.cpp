#include "mainwindow.h"
#include "drfxbuilder.h"
#include "drfxprogressdialog.h"
#include "drfxtypes.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMap>
#include <QMessageBox>
#include <QScreen>
#include <QSplitter>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <QTimer>

Q_DECLARE_METATYPE(QTreeWidgetItem *);
Q_DECLARE_METATYPE(QTableWidgetItem *);

#ifdef Q_OS_MACOS
#define FUSION_MACRO_PATH "${HOME}/Library/Application Support/Blackmagic Design/Fusion/Macros"
#define FUSION_TEMPLATE_PATH "${HOME}/Library/Application Support/Blackmagic Design/Fusion/Templates"
#define DAVINCI_MACRO_PATH "${HOME}/Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Macros"
#define DAVINCI_TEMPLATE_PATH "${HOME}/Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Templates"
#endif

#ifdef Q_OS_WINDOWS
#define FUSION_MACRO_PATH "TODO"
#define DAVINCI_MACRO_PATH "TODO"
#endif

#ifdef Q_OS_LINUX
#define FUSION_MACRO_PATH "TODO"
#define DAVINCI_MACRO_PATH "TODO"
#endif

inline static QString configFile()
{
    QString path = QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::ConfigLocation);
    return path.append(QDir::separator()).append("eof_drfx_builder.conf");
}

inline static QString bundleStuctureFile()
{
    QString path = QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::ConfigLocation);
    return path.append(QDir::separator()).append("eof_drfx_structure.conf");
}

inline static QString picturePath()
{
    return QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::PicturesLocation);
}

inline static QString bundleOutputName()
{
    QString path = QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::DocumentsLocation);
    return path.append(QDir::separator()).append("bundle.drfx");
}

inline static QString homePath(const QString &path = "")
{
    QString home = QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::HomeLocation);
    if (path.isEmpty())
        return home;
    QString p = path;
    return p.replace("${HOME}", home, Qt::CaseInsensitive);
}

inline static QString toHash(const QString &nodePath)
{
    QByteArray hash = QCryptographicHash::hash( //
        nodePath.toLocal8Bit(),                 //
        QCryptographicHash::Algorithm::Sha1);
    return hash.toHex();
}

inline static QString shortenText(const QString &value, int max)
{
    QString result = value;
    if (result.length() > max && (max / 2) > 0) {
        result = result.left(max / 2) + " ... " + result.right(max / 2);
    }
    return result;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(configFile(), QSettings::Format::NativeFormat)
    , m_macroPath(homePath(DAVINCI_MACRO_PATH))
    , m_iconPath(picturePath())
    , m_outputName(bundleOutputName())
    , m_installPath(homePath(DAVINCI_TEMPLATE_PATH))
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/assets/png/drfxbuilder.png"));
    setWindowTitle(qApp->applicationDisplayName());

    ui->statusbar->setSizeGripEnabled(true);
    ui->statusbar->showMessage(tr("%1 %2 Copyright (c) 2025 by EoF Software Labs") //
                                   .arg(qApp->applicationDisplayName(), qApp->applicationVersion()),
                               20000);

    // center window on primary screen
    QScreen *screen = qApp->primaryScreen();
    int width = m_settings.value("window.width", geometry().width()).toUInt();
    int hight = m_settings.value("window.height", geometry().height()).toUInt();
    if (width > 0 && width < screen->size().width() && hight > 0 && hight < screen->size().height()) {
        uint centerX = screen->size().width() / 2 - width / 2;
        uint centerY = screen->size().height() / 2 - hight / 2;
        setGeometry(centerX, centerY, width, hight);
    }

    QSplitter *splitter = new QSplitter(Qt::Horizontal, ui->centralwidget);
    ui->centralwidget->layout()->addWidget(splitter);
    splitter->insertWidget(0, ui->pnlBundle);
    splitter->insertWidget(1, ui->pnlContent);
    splitter->setHandleWidth(10);

    int spleft = m_settings.value("splitter.left", width / 2).toInt();
    int spright = m_settings.value("splitter.right", width / 2).toInt();
    splitter->setSizes(QList<int>() << spleft << spright);

    connect(splitter, &QSplitter::splitterMoved, this, [this, splitter](int, int) {
        m_settings.setValue("splitter.left", splitter->sizes().at(0));
        m_settings.setValue("splitter.right", splitter->sizes().at(1));
    });

    connect(qApp, &QApplication::aboutToQuit, this, [this] {
        saveBundleStructure();
        m_settings.setValue("window.width", geometry().width()); //
        m_settings.setValue("window.height", geometry().height());
        // write app settings
        m_settings.sync();
        if (m_settings.status() != QSettings::NoError) {
            qCritical("Unable to write settings file.");
        }
        // terminate scanner thread
        //if (m_thread != nullptr) {
        //    m_thread->requestInterruption();
        //    m_thread->wait();
        //}
    });

    m_appType = m_settings.value("app.type", 0).toUInt();
    m_outputName = m_settings.value("output.name", m_outputName).toString();
    m_installPath = m_settings.value("install.path", m_installPath).toString();
    m_macroPath = m_settings.value("macro.path", m_macroPath).toString();
    m_iconPath = m_settings.value("icon.path", m_iconPath).toString();

    // async update ui fields
    checkBlackmagic();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_edFusionMacro_textChanged(const QString &value)
{
    ui->edFusionMacro->setStatusTip(shortenText(value, 80));
    const QFileInfo fi(value);
    m_macroPath = fi.absolutePath();
    m_settings.setValue("macro.file", value);
    m_settings.setValue("macro.path", m_macroPath);
    checkInputFields();
    updateTargetInfo();
}

void MainWindow::on_edFusionMacro_textEdited(const QString &value)
{
    on_edFusionMacro_textChanged(value);
}

void MainWindow::on_edIconFile_textChanged(const QString &value)
{
    ui->edIconFile->setStatusTip(shortenText(value, 80));
    const QFileInfo fi(value);
    m_iconPath = fi.absolutePath();
    m_settings.setValue("icon.file", value);
    m_settings.setValue("icon.path", m_iconPath);
    checkInputFields();
    updateTargetInfo();
}

void MainWindow::on_edIconFile_textEdited(const QString &value)
{
    on_edIconFile_textChanged(value);
}

void MainWindow::on_edCompany_textChanged(const QString &value)
{
    m_settings.setValue("company", value);
    checkInputFields();
    updateTargetInfo();
}

void MainWindow::on_edCompany_textEdited(const QString &value)
{
    on_edCompany_textChanged(value);
}

void MainWindow::on_edProduct_textChanged(const QString &value)
{
    m_settings.setValue("product", value);
    checkInputFields();
    updateTargetInfo();
}

void MainWindow::on_edProduct_textEdited(const QString &value)
{
    on_edProduct_textChanged(value);
}

void MainWindow::on_cbBundleNode_activated(int index)
{
    QVariant v = ui->cbBundleNode->itemData(index, Qt::UserRole);
    if (v.isNull() || !v.isValid()) {
        return;
    }
    m_settings.setValue("nodesel", index);
    updateTargetInfo();
}

void MainWindow::on_cbBundleNode_currentIndexChanged(int)
{
    updateTargetInfo();
}

void MainWindow::on_tvBundleStruct_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    const QVariant v = current->data(0, Qt::ItemDataRole::UserRole);
    if (v.isNull() || !v.isValid()) {
        return;
    }
    const TNodeData nd = v.value<TNodeData>();
    switch (nd.type) {
        case TNodeType::Company: {
            ui->edCompany->setText(nd.name);
            cleanupTableView();
            break;
        }
        // add to table view if product node
        case TNodeType::Product: {
            ui->edProduct->setText(nd.name);
            fillTableView(current);
            break;
        }
        // select item in table view
        case TNodeType::FileItem: {
            fillTableView(current->parent());
            selectTableRow(current);
            break;
        }
        // cleanup table
        default: {
            selectComboBoxItem(current);
            cleanupTableView();
            break;
        }
    }
}

// select node in tree view
void MainWindow::on_twNodeList_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *)
{
    // on delete rows from table view, parameter can be NULL
    if (!current) {
        return;
    }

    const QVariant data = current->data(Qt::ItemDataRole::UserRole);
    if (data.isNull() || !data.isValid()) {
        return;
    }
    const TNodeData nd = data.value<TNodeData>();
    switch (nd.type) {
        case TNodeType::Company: {
            ui->edCompany->setText(nd.name);
            break;
        }
        case TNodeType::Product: {
            ui->edProduct->setText(nd.name);
            break;
        }
        case TNodeType::FileItem: {
            QTreeWidgetItem *twi;
            // iterate overall treeview
            if ((twi = findNodeByHash(ui->tvBundleStruct->topLevelItem(0), nd.hash))) {
                ui->tvBundleStruct->setCurrentItem(twi);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void MainWindow::on_twNodeList_itemClicked(QTableWidgetItem *item)
{
    QTreeWidgetItem *twi;
    bool enabled = false;

    QVariant v = item->data(Qt::ItemDataRole::UserRole);
    if (!v.isNull() && v.isValid()) {
        switch (ui->twNodeList->column(item)) {
            case 0: {
                enabled = v.value<TNodeData>().type == TNodeType::FileItem;
                break;
            }
            case 1: {
                if ((twi = v.value<QTreeWidgetItem *>())) {
                    v = twi->data(0, Qt::ItemDataRole::UserRole);
                    if (!v.isNull() && v.isValid()) {
                        enabled = v.value<TNodeData>().type == TNodeType::FileItem;
                    }
                }
                break;
            }
        }
    }
    ui->pbDelete->setEnabled(enabled);
}

void MainWindow::on_pbNewBundle_clicked()
{
    if (QMessageBox::question(this,
                              qApp->applicationDisplayName(), //
                              tr("Do you want to create new bundle?")) == QMessageBox::Yes) {
        resetBundleStructure(ui->tvBundleStruct->topLevelItem(0));
        ui->pbBuildDRFX->setDefault(false);
        ui->pbBuildDRFX->setEnabled(false);
    }
}

void MainWindow::on_pbSelectMacro_clicked()
{
    QFileDialog d(this);

    connect(&d, &QFileDialog::fileSelected, this, [this](const QString &file) {
        qDebug("File selected: %s", qPrintable(file));
        on_edFusionMacro_textEdited(file);
        ui->edFusionMacro->setText(file);
        ui->edFusionMacro->setFocus();
    });
    connect(&d, &QFileDialog::directoryEntered, this, [this](const QString &directory) {
        qDebug("Directory entered: %s", qPrintable(directory));
        m_settings.setValue("macro.path", directory);
        m_macroPath = directory;
    });

    d.setWindowTitle(tr("Select Fusion macro file"));
    d.setWindowFilePath(m_macroPath);
    d.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    d.setFileMode(QFileDialog::FileMode::ExistingFile);
    d.setDirectory(m_macroPath);
    d.setLabelText(QFileDialog::DialogLabel::FileName, tr("Fusion macro file"));
    d.setOption(QFileDialog::Option::ReadOnly);
    d.setNameFilters(QStringList() << "*.setting");
    d.setDefaultSuffix(".setting");
    d.exec();
}

void MainWindow::on_pbSelectIcon_clicked()
{
    QFileDialog d(this);

    connect(&d, &QFileDialog::fileSelected, this, [this](const QString &file) {
        qDebug("File selected: %s", qPrintable(file));
        on_edIconFile_textEdited(file);
        ui->edIconFile->setText(file);
        ui->edIconFile->setFocus();
    });
    connect(&d, &QFileDialog::directoryEntered, this, [this](const QString &directory) {
        qDebug("Directory entered: %s", qPrintable(directory));
        m_settings.setValue("icon.path", directory);
        m_iconPath = directory;
    });

    d.setWindowTitle(tr("Select icon file"));
    d.setWindowFilePath(m_iconPath);
    d.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    d.setFileMode(QFileDialog::FileMode::ExistingFile);
    d.setDirectory(m_iconPath);
    d.setLabelText(QFileDialog::DialogLabel::FileName, tr("Icon file"));
    d.setOption(QFileDialog::Option::ReadOnly);
    d.setNameFilters(QStringList() << "*.png" << "*.*");
    d.setDefaultSuffix(".setting");
    d.exec();
}

void MainWindow::on_pbImport_clicked()
{
    QString path = ui->cbBundleNode->currentText();
    QVariant v = ui->cbBundleNode->currentData(Qt::UserRole);
    if (v.isNull() || !v.isValid()) {
        return;
    }

    QTreeWidgetItem *node;
    if (!(node = v.value<QTreeWidgetItem *>())) {
        return;
    }

    addToNode(path, node);
}

// select node in tree view
void MainWindow::on_pbDelete_clicked()
{
    QTreeWidgetItem *twi = nullptr;
    QTreeWidgetItem *parent = nullptr;
    QTableWidgetItem *item = ui->twNodeList->currentItem();
    QVariant v = item->data(Qt::ItemDataRole::UserRole);
    if (!v.isNull() && v.isValid()) {
        switch (ui->twNodeList->column(item)) {
            case 0: {
                const TNodeData nd = v.value<TNodeData>();
                twi = findNodeByHash(ui->tvBundleStruct->topLevelItem(0), nd.hash);
                break;
            }
            case 1: {
                twi = v.value<QTreeWidgetItem *>();
                break;
            }
        }
    }

    if (twi) {
        if (QMessageBox::question(this,
                                  qApp->applicationDisplayName(), //
                                  tr("Do you want to delete object: %1").arg(twi->text(0))) == QMessageBox::Yes) {
            parent = twi->parent();
            parent->removeChild(twi);
            delete twi;
            int row = ui->twNodeList->row(item);
            ui->twNodeList->removeRow(row);
            // check BUG of QT: will not remove always. we call cleanup
            if (ui->twNodeList->rowCount() > parent->childCount()) {
                cleanupTableView();
            }
        }
    }
}

void MainWindow::on_pbBuildDRFX_clicked()
{
    if (QMessageBox::question(this,
                              qApp->applicationDisplayName(), //
                              tr("Do you want to build DRFX bundle?\nBundle: %1").arg(m_outputName)) == QMessageBox::Yes) {
        const QFileInfo fi(m_outputName);
        DRFXBuilder *builder = new DRFXBuilder(m_outputName, this);
        connect(builder, &DRFXBuilder::buildStarted, this, &MainWindow::onBuildStarted, Qt::QueuedConnection);
        connect(builder, &DRFXBuilder::buildComplete, this, &MainWindow::onBuildComplete, Qt::QueuedConnection);
        connect(builder, &DRFXBuilder::buildError, this, &MainWindow::onBuildError, Qt::QueuedConnection);

        QFileDialog d(this);
        connect(&d, &QFileDialog::fileSelected, this, [this, builder](const QString &file) {
            qDebug("Directory selected: %s", qPrintable(file));
            m_settings.setValue("output.name", file);
            builder->setOutputName(file);
        });
        connect(&d, &QFileDialog::directoryEntered, this, [](const QString &directory) { //
            qDebug("Directory entered: %s", qPrintable(directory));
        });

        d.setWindowTitle(tr("Save DRFX bundle file"));
        d.setWindowFilePath(fi.absolutePath());
        d.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
        d.setFileMode(QFileDialog::FileMode::AnyFile);
        d.setDirectory(fi.absolutePath());
        d.setLabelText(QFileDialog::DialogLabel::FileName, tr("Bundle file:"));
        d.setLabelText(QFileDialog::DialogLabel::FileType, "Extension (.drfx):");
        d.setOption(QFileDialog::DontConfirmOverwrite, true);
        d.setOption(QFileDialog::DontUseNativeDialog, false);
        d.setNameFilters(QStringList() << "*.drfx" << "*.*");
        d.setHistory(QStringList() << fi.fileName());
        d.setDefaultSuffix(".drfx");
        if (d.exec() == QFileDialog::Accepted) {
            builder->build(ui->tvBundleStruct->topLevelItem(0));
        }
    }
}

void MainWindow::on_pbInstall_clicked()
{
    QFileInfo srcfi(m_outputName);
    QString fname = QDir::toNativeSeparators(m_installPath + QDir::separator() + srcfi.fileName());

    if (QFile::exists(fname)) {
        QFileInfo tgtfi(fname);
        QDateTime tdt = tgtfi.fileTime(QFile::FileTime::FileModificationTime);
        QDateTime sdt = srcfi.fileTime(QFile::FileTime::FileModificationTime);
        if (tgtfi.size() == srcfi.size() && tdt == sdt) {
            QMessageBox::critical(this,
                                  qApp->applicationDisplayName(), //
                                  tr("Target file is identical to source file."));
            return;
        }
    }

    DRFXProgressDialog *dlg = new DRFXProgressDialog(this);
    dlg->setMessage(tr("Please wait, install bundle: %1").arg(srcfi.fileName()));
    dlg->setRange(0, srcfi.size());
    dlg->run(
        [this, fname](DRFXProgressDialog *p, QThread *t) {
            QFile srcf(m_outputName);
            if (!srcf.open(QFile::ReadOnly)) {
                p->setError(tr("Could not open file: %1").arg(m_outputName));
                return;
            }

            QFile tgtf(fname);
            if (!tgtf.open(QFile::WriteOnly | QFile::Truncate)) {
                srcf.close();
                p->setError(tr("Could not create file: %1").arg(fname));
                return;
            }

            const qsizetype chunksize = 16384;
            qsizetype bytesRead = 0;
            QByteArray buffer;
            buffer = srcf.read(chunksize);
            while (buffer.length() > 0 && !t->isInterruptionRequested()) {
                t->yieldCurrentThread();
                p->setValue(bytesRead);
                bytesRead += buffer.length();
                tgtf.write(buffer);
                buffer = srcf.read(chunksize);
            }

            srcf.close();
            tgtf.flush();
            tgtf.close();
        },
        [](DRFXProgressDialog *p) { p->deleteLater(); });
}

void MainWindow::onBuildError(DRFXBuilder *builder, const QString &message)
{
    QMessageBox::critical(this, qApp->applicationDisplayName(), message);
    ui->pbBuildDRFX->setEnabled(true);
    builder->disconnect(this);
    builder->deleteLater();
}

void MainWindow::onBuildStarted(DRFXBuilder *)
{
    ui->pbBuildDRFX->setEnabled(false);
}

void MainWindow::onBuildComplete(DRFXBuilder *builder, const QString &fileName)
{
    QMessageBox::information(this,
                             qApp->applicationDisplayName(), //
                             tr("Bundle file '%1' successfully created.").arg(fileName));
    ui->pbBuildDRFX->setEnabled(true);
    builder->disconnect(this);
    builder->deleteLater();
    m_outputName = fileName;
    checkOutputExist();
}

inline void MainWindow::postInitUi()
{
    ui->tvBundleStruct->expandAll();
    ui->edFusionMacro->setPlaceholderText(tr("Enter macro file name"));
    ui->edFusionMacro->setText(m_settings.value("macro.file").toString());
    ui->edIconFile->setPlaceholderText(tr("Enter icon file name"));
    ui->edIconFile->setText(m_settings.value("icon.file").toString());
    ui->edCompany->setPlaceholderText(tr("Enter company name"));
    ui->edCompany->setText(m_settings.value("company").toString());
    ui->edProduct->setPlaceholderText(tr("Enter product name"));
    ui->edProduct->setText(m_settings.value("product").toString());

    // fill target combobox
    int nodesel = m_settings.value("nodesel", 0).toInt();
    if (ui->tvBundleStruct->topLevelItemCount() != 0) {
        cbxAddBundleItems(ui->tvBundleStruct->topLevelItem(0));
        if (ui->cbBundleNode->count() > nodesel) {
            ui->cbBundleNode->setCurrentIndex(nodesel);
            on_cbBundleNode_activated(nodesel);
        }
    }

    // restore bundle */
    loadBundleStructure();

    // any content exist?
    if (checkBundleContent(ui->tvBundleStruct->topLevelItem(0))) {
        ui->pbImport->setDefault(false);
        ui->pbBuildDRFX->setEnabled(true);
        ui->pbBuildDRFX->setDefault(true);
    }

    // input field restored?
    checkInputFields();

    // already generated ?
    checkOutputExist();
}

inline void MainWindow::updateTargetInfo()
{
    QString text = ui->cbBundleNode->currentText();
    if (!ui->edCompany->text().isEmpty()) {
        text = text.append("/").append(ui->edCompany->text());
    }
    if (!ui->edProduct->text().isEmpty()) {
        text = text.append("/").append(ui->edProduct->text());
    }
    ui->txTargetPath->setText(text);
}

inline void MainWindow::checkOutputExist()
{
    if (QFile::exists(m_outputName)) {
        ui->pbImport->setDefault(false);
        ui->pbBuildDRFX->setDefault(false);
        ui->pbInstall->setEnabled(true);
        ui->pbInstall->setDefault(true);
        ui->txBundleFile->setText(tr("Bundle file: %1").arg(shortenText(m_outputName, 80)));
    }
}

inline void MainWindow::checkBlackmagic()
{
    // check which installed Fusion or Davinci or both
    if (m_appType == 0) {
        QTimer::singleShot(50, this, [this] {
            const QIcon icon(":/assets/dfrxbuilder.iconset/icon_32x32.png");
            const QDir dbmf(homePath(FUSION_MACRO_PATH));
            const QDir dbmd(homePath(DAVINCI_MACRO_PATH));
            const QDir dbif(homePath(FUSION_TEMPLATE_PATH));
            const QDir dbid(homePath(DAVINCI_TEMPLATE_PATH));
            int flags = 0;

            if (dbmf.exists()) {
                flags |= 0x01;
            }

            if (dbmd.exists()) {
                flags |= 0x02;
            }

            // both: ask user
            if ((flags & 0x03) == 0x03) {
                QMessageBox mb(this);
                mb.setTextFormat(Qt::TextFormat::PlainText);
                mb.setText(tr("Davinci Resolve and Fusion found.\n" //
                              "Please select which one to use."));
                mb.setIconPixmap(icon.pixmap(32, 32));
                mb.setStandardButtons(QMessageBox::Button::Ok |  //
                                      QMessageBox::Button::Yes | //
                                      QMessageBox::Button::Cancel);
                mb.setButtonText(QMessageBox::Button::Ok, tr("Davinci Resolve"));
                mb.setButtonText(QMessageBox::Button::Yes, tr("Fusion"));
                mb.setButtonText(QMessageBox::Button::Cancel, tr("Close"));
                mb.setDefaultButton(QMessageBox::Button::Ok);
                mb.setWindowTitle(qApp->applicationDisplayName());
                switch (mb.exec()) {
                    case QMessageBox::Button::Yes: {
                        m_macroPath = dbmf.absolutePath();
                        m_installPath = dbif.absolutePath();
                        m_appType = 1;
                        break;
                    }
                    case QMessageBox::Button::Ok: {
                        m_macroPath = dbmd.absolutePath();
                        m_installPath = dbid.absolutePath();
                        m_appType = 2;
                        break;
                    }
                    default: {
                        qApp->quit();
                        return;
                    }
                }
            }
            // fusion
            else if ((flags & 0x01) == 0x01) {
                m_macroPath = dbmf.absolutePath();
                m_installPath = dbif.absolutePath();
            }
            // davinci
            else if ((flags & 0x02) == 0x02) {
                m_macroPath = dbmd.absolutePath();
                m_installPath = dbid.absolutePath();
            }
            m_settings.setValue("macro.path", m_macroPath);
            m_settings.setValue("install.path", m_installPath);
            m_settings.setValue("app.type", m_appType);
            postInitUi();
        });
    } else {
        postInitUi();
    }
}

inline void MainWindow::checkInputFields()
{
    bool enabled = true;
    enabled &= !ui->edFusionMacro->text().isEmpty();
    enabled &= !ui->edIconFile->text().isEmpty();
    enabled &= !ui->edCompany->text().isEmpty();
    enabled &= !ui->edProduct->text().isEmpty();
    ui->pbImport->setEnabled(enabled);
    ui->pbImport->setDefault(enabled);
}

// gather all static treeview items
inline void MainWindow::cbxAddBundleItems(QTreeWidgetItem *node, const QString &prefix)
{
    const QVariant vt = node->data(0, Qt::DisplayRole);
    if (vt.isNull() || !vt.isValid()) {
        return;
    }

    QString tpath = prefix;
    QString title = vt.toString();

    if (!tpath.isEmpty()) {
        title = tpath + "/" + vt.toString();
    }

    const QVariant data = node->data(0, Qt::UserRole);
    if (data.isNull() || !data.isValid()) {
        TNodeData nd = {Static, toHash(title), title, vt.toString()};
        node->setData(0, Qt::UserRole, QVariant::fromValue(nd));
    }

    // add selection item
    if (node != ui->tvBundleStruct->topLevelItem(0)) {
        ui->cbBundleNode->addItem(title, QVariant::fromValue(node));
    }

    for (int i = 0; i < node->childCount(); i++) {
        cbxAddBundleItems(node->child(i), title);
    }
}

inline bool MainWindow::checkBundleContent(QTreeWidgetItem *node)
{
    bool result = false;
    for (int i = 0; i < node->childCount(); i++) {
        QTreeWidgetItem *child = node->child(i);
        if (child->childCount() > 0) {
            const QVariant data = child->data(0, Qt::UserRole);
            if (data.isNull() || !data.isValid()) {
                continue;
            }
            const TNodeData nd = data.value<TNodeData>();
            if (nd.type == TNodeType::Product) {
                result = true;
                break;
            }
            if (checkBundleContent(child)) {
                result = true;
                break;
            }
        }
    }
    return result;
}

inline QTreeWidgetItem *MainWindow::findNodeByHash(QTreeWidgetItem *item, const QString &hash)
{
    QTreeWidgetItem *result = nullptr;
    for (int i = 0; i < item->childCount(); i++) {
        QTreeWidgetItem *child = item->child(i);
        const QVariant v = child->data(0, Qt::UserRole);
        if (v.isNull() || !v.isValid()) {
            continue;
        }
        const TNodeData nd = v.value<TNodeData>();
        const QString _hash = toHash(nd.path);
        if (_hash == hash) {
            return child;
        }
        if (child->childCount() > 0) {
            result = findNodeByHash(child, hash);
            if (result) {
                return result;
            }
        }
    }
    return result;
}

inline void MainWindow::saveBundleStructure()
{
    if (ui->tvBundleStruct->topLevelItemCount() == 0) {
        return;
    }

    QFile f(bundleStuctureFile());
    if (!f.open(QFile::Truncate | QFile::ReadWrite)) {
        QMessageBox::critical(this, qApp->applicationDisplayName(), tr("Unable to save bundle structure."));
        return;
    }

    QJsonObject root;
    bundleToJson(root, ui->tvBundleStruct->topLevelItem(0));

    if (!root.isEmpty()) {
        const QJsonDocument jdoc(root);
        f.write(jdoc.toJson(QJsonDocument::Indented));
        //f.write(jdoc.toJson(QJsonDocument::Compact));
        f.flush();
        f.close();
    }
}

inline QTreeWidgetItem *MainWindow::addCompanyNode(QTreeWidgetItem *node, const QString &name, const QString &path)
{
    const TNodeData data = {TNodeType::Company, toHash(path), path, name};
    QTreeWidgetItem *child = new QTreeWidgetItem(node, QTreeWidgetItem::Type);
    child->setData(0, Qt::DisplayRole, QVariant::fromValue(data.name));
    child->setData(0, Qt::UserRole, QVariant::fromValue(data));
    node->addChild(child);
    return child;
}

inline QTreeWidgetItem *MainWindow::addProductNode(QTreeWidgetItem *node, const QString &name, const QString &path)
{
    const TNodeData data = {TNodeType::Product, toHash(path), path, name};
    QTreeWidgetItem *child = new QTreeWidgetItem(node, QTreeWidgetItem::Type);
    child->setData(0, Qt::DisplayRole, QVariant::fromValue(data.name));
    child->setData(0, Qt::UserRole, QVariant::fromValue(data));
    node->addChild(child);
    return child;
}

inline int MainWindow::addFileNode(QTreeWidgetItem *node, int errorBits, const QString &fileName)
{
    // add icon file name
    const QFileInfo info = QFileInfo(fileName);
    const QString fname = info.fileName();
    const TNodeData data = {TNodeType::FileItem, toHash(info.filePath()), info.filePath(), fname};
    QTreeWidgetItem *child;

    if (findNodeByHash(ui->tvBundleStruct->topLevelItem(0), data.hash)) {
        QMessageBox::critical(this,
                              qApp->applicationDisplayName(), //
                              tr("Object '%1' already exist.").arg(fname));
        return errorBits;
    } else {
        child = new QTreeWidgetItem(node, QTreeWidgetItem::Type);
        child->setData(0, Qt::UserRole, QVariant::fromValue(data));
        child->setData(0, Qt::DisplayRole, data.name);
        node->addChild(child);
    }

    return 0;
}

inline void MainWindow::addToNode(const QString &path, QTreeWidgetItem *root)
{
    QTreeWidgetItem *child, *comp;
    QString _path = path;
    int state = 0;

    if (root == ui->tvBundleStruct->topLevelItem(0)) {
        QMessageBox::critical(this,
                              qApp->applicationDisplayName(), //
                              tr("Items below 'Edit' node are not allowed."));
    }

    _path += "/" + ui->edCompany->text();
    if (!(comp = findNodeByHash(root, toHash(_path)))) {
        comp = addCompanyNode(root, ui->edCompany->text(), _path);
        state = 0xe0;
    }
    _path += "/" + ui->edProduct->text();
    if (!(child = findNodeByHash(comp, toHash(_path)))) {
        child = addProductNode(comp, ui->edProduct->text(), _path);
        state = 0xe0;
    }

#if 0
    if (!(child = findCompanyAndProduct(root))) {
        child = addProductNode(root, ui->edCompany->text(), path);
        child = addProductNode(child, ui->edProduct->text(), path + "/" + ui->edProduct->text());
        state = 0xf0;
    }
#endif

    state |= addFileNode(child, 0x01, ui->edFusionMacro->text());
    state |= addFileNode(child, 0x02, ui->edIconFile->text());

    if ((state & 0x01) && (state & 0x02) && (state & 0xf0)) {
        QTreeWidgetItem *parent = child->parent();
        parent->removeChild(child);
        root->removeChild(parent);
        delete child;
        delete parent;
    } else {
        ui->statusbar->showMessage(tr("Fusion added to bundle."), 5000);
    }

    if (checkBundleContent(ui->tvBundleStruct->topLevelItem(0))) {
        ui->pbBuildDRFX->setEnabled(true);
        ui->pbBuildDRFX->setDefault(true);
        ui->pbImport->setDefault(false);
    }
}

inline void MainWindow::bundleToJson(QJsonObject &json, QTreeWidgetItem *item)
{
    if (item->childCount() == 0) {
        return;
    }

    for (int i = 0; i < item->childCount(); i++) {
        QTreeWidgetItem *child = item->child(i);
        if (child->childCount() > 0) {
            bundleToJson(json, child);
        }

        // child data object
        const QVariant vChild = child->data(0, Qt::ItemDataRole::UserRole);
        if (vChild.isNull() || !vChild.isValid()) {
            continue;
        }
        const TNodeData ndChild = vChild.value<TNodeData>();

        // skip if not file node
        if (ndChild.type == TNodeType::Static || ndChild.type == TNodeType::None) {
            continue;
        }

        // parents data object
        const QVariant vParent = item->data(0, Qt::ItemDataRole::UserRole);
        if (vParent.isNull() || !vParent.isValid()) {
            continue;
        }
        const TNodeData ndParent = vParent.value<TNodeData>();

        const QString hash = toHash(ndChild.path);
        const QString key = QStringLiteral("%1/%2").arg(ndChild.type).arg(hash);

        QJsonObject jo;
        jo["node.parent"] = ndParent.hash;
        //jo["data.hash"] = QJsonValue(hash);
        jo["node.type"] = QJsonValue(ndChild.type);
        jo["data.name"] = QJsonValue(ndChild.name);
        jo["data.path"] = QJsonValue(ndChild.path);

        // store to node object
        json[key] = jo;
    }
}

inline bool MainWindow::loadBundleStructure()
{
    QFile f(bundleStuctureFile());
    if (!f.open(QFile::ReadOnly) || f.size() == 0) {
        return false; // may not exist (first run)
    }

    QJsonParseError error;
    QJsonDocument jdoc = QJsonDocument::fromJson(f.readAll(), &error);
    f.close();

    if (error.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, qApp->applicationDisplayName(), error.errorString());
        return false;
    }

    const QJsonObject jroot = jdoc.object();
    if (jroot.isEmpty()) {
        QMessageBox::critical(this, qApp->applicationDisplayName(), tr("Could not load bundle structure."));
        return false;
    }

    QTreeWidgetItem *parent = nullptr;
    QTreeWidgetItem *root = ui->tvBundleStruct->topLevelItem(0);
    const QStringList keys = jroot.keys();
    foreach (const QString key, keys) {
        const QJsonObject jo = jroot[key].toObject();

        if (!jo.contains("node.parent")) {
            QMessageBox::critical(this, qApp->applicationDisplayName(), tr("Node parent missing in bundle file."));
            return false;
        }
        const QString nodeParent = jo["node.parent"].toString();

        if (!jo.contains("node.type")) {
            QMessageBox::critical(this, qApp->applicationDisplayName(), tr("Node type missing in bundle file."));
            return false;
        }
        const int nodeType = jo["node.type"].toInt();

        if (!jo.contains("data.name")) {
            QMessageBox::critical(this, qApp->applicationDisplayName(), tr("Node name missing in bundle file."));
            return false;
        }
        const QString nodeName = jo["data.name"].toString();

        if (!jo.contains("data.path")) {
            QMessageBox::critical(this, qApp->applicationDisplayName(), tr("Node path missing in bundle file."));
            return false;
        }
        const QString nodePath = jo["data.path"].toString();

        switch (nodeType) {
            case TNodeType::Company: {
                if ((parent = findNodeByHash(root, nodeParent))) {
                    if (!findNodeByHash(parent, toHash(nodePath))) {
                        addCompanyNode(parent, nodeName, nodePath);
                    }
                }
                break;
            }
            case TNodeType::Product: {
                if ((parent = findNodeByHash(root, nodeParent))) {
                    if (!findNodeByHash(parent, toHash(nodePath))) {
                        addProductNode(parent, nodeName, nodePath);
                    }
                }
                break;
            }
            case TNodeType::FileItem: {
                if ((parent = findNodeByHash(root, nodeParent))) {
                    if (addFileNode(parent, 0x04, nodePath) != 0) {
                        QMessageBox::critical(this,
                                              qApp->applicationDisplayName(), //
                                              tr("Object '%1' already exist.").arg(nodeName));
                        return false;
                    }
                }
                break;
            }
            default: {
                QMessageBox::critical(this, qApp->applicationDisplayName(), tr("Invalid node type detected."));
                return false;
            }
        }
    }
    return true;
}

// select combobox item by given tree node
inline void MainWindow::selectComboBoxItem(QTreeWidgetItem *node)
{
    for (int i = 0; i < ui->cbBundleNode->count(); i++) {
        const QVariant v = ui->cbBundleNode->itemData(i, Qt::UserRole);
        if (v.isNull() || !v.isValid()) {
            continue;
        }
        if (node == v.value<QTreeWidgetItem *>()) {
            ui->cbBundleNode->setCurrentIndex(i);
            break; // fini loop
        }
    }
}

// select table row by given tree node
inline void MainWindow::selectTableRow(QTreeWidgetItem *node)
{
    QTableWidgetItem *item;
    for (int i = 0; i < ui->twNodeList->rowCount(); i++) {
        // column 1 holds ptr to node
        if ((item = ui->twNodeList->item(i, 1))) {
            const QVariant v = item->data(Qt::UserRole);
            if (v.isNull() || !v.isValid()) {
                continue;
            }
            if (v.value<QTreeWidgetItem *>() == node) {
                ui->twNodeList->setCurrentItem(item);
            }
        }
    }
}

// add file items from treeview
inline void MainWindow::fillTableView(QTreeWidgetItem *node)
{
    // cleanup first
    cleanupTableView();

    // add items from tree view
    for (int i = 0; i < node->childCount(); i++) {
        QTreeWidgetItem *child = node->child(i);
        const QVariant data = child->data(0, Qt::ItemDataRole::UserRole);
        if (data.isNull() || !data.isValid()) {
            continue;
        }
        const TNodeData nd = data.value<TNodeData>();
        if (nd.type != TNodeType::FileItem) {
            continue;
        }

        // add row
        ui->twNodeList->setRowCount(ui->twNodeList->rowCount() + 1);

        // assing name column
        QTableWidgetItem *item;
        item = new QTableWidgetItem(QTableWidgetItem::Type);
        item->setData(Qt::UserRole, QVariant::fromValue(nd));
        item->setData(Qt::DisplayRole, nd.name);
        ui->twNodeList->setItem(i, 0, item);

        // assing path column
        item = new QTableWidgetItem(QTableWidgetItem::Type);
        item->setData(Qt::UserRole, QVariant::fromValue(child));
        item->setData(Qt::DisplayRole, nd.path);
        ui->twNodeList->setItem(i, 1, item);
    }
}

// delete all rows in table view
inline void MainWindow::cleanupTableView()
{
    while (ui->twNodeList->rowCount() > 0) {
        QTableWidgetItem *a1 = ui->twNodeList->takeItem(0, 0);
        QTableWidgetItem *a2 = ui->twNodeList->takeItem(0, 1);
        ui->twNodeList->removeRow(0);
        delete a1;
        delete a2;
    }
    ui->pbDelete->setEnabled(false);
}

inline void MainWindow::resetBundleStructure(QTreeWidgetItem *node)
{
    QList<QTreeWidgetItem *> removeList;
    for (int i = 0; i < node->childCount(); i++) {
        QTreeWidgetItem *child = node->child(i);
        if (child->childCount() > 0) {
            resetBundleStructure(child);
        }
        const QVariant v = child->data(0, Qt::ItemDataRole::UserRole);
        if (v.isNull() || !v.isValid()) {
            continue;
        }
        TNodeData nd = v.value<TNodeData>();
        if (nd.type == TNodeType::Static || nd.type == TNodeType::None) {
            continue;
        }
        removeList.append(child);
    }
    while (!removeList.isEmpty()) {
        QTreeWidgetItem *item = removeList.takeFirst();
        node->removeChild(item);
        delete item;
    }
}
