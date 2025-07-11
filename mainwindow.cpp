#include "mainwindow.h"
#include "drfxbuilder.h"
#include "drfxtypes.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QCryptographicHash>
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
#include <QSplitter>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <QTimer>

Q_DECLARE_METATYPE(QTreeWidgetItem *);
Q_DECLARE_METATYPE(QTableWidgetItem *);

#ifdef Q_OS_MACOS
#define FUSION_MACRO_PATH "/Users/beschric/Library/Application Support/Blackmagic Design/Fusion/Macros"
#define DAVINCI_MACRO_PATH "/Users/beschric/Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Macros"
#endif

#ifdef Q_OS_WINDOWS
#define FUSION_MACRO_PATH "/Users/beschric/Library/Application Support/Blackmagic Design/Fusion/Macros"
#define DAVINCI_MACRO_PATH "/Users/beschric/Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Macros"
#endif

#ifdef Q_OS_LINUX
#define FUSION_MACRO_PATH "/Users/beschric/Library/Application Support/Blackmagic Design/Fusion/Macros"
#define DAVINCI_MACRO_PATH "/Users/beschric/Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Macros"
#endif

inline static QString configFile()
{
    QString path = QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::ConfigLocation);
    return path.append(QDir::separator()).append(".eof_drfx_builder.conf");
}

inline static QString bundleStuctureFile()
{
    QString path = QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::ConfigLocation);
    return path.append(QDir::separator()).append(".eof_drfx_structure.conf");
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

inline static QString toHash(const QString &nodePath)
{
    QByteArray hash = QCryptographicHash::hash( //
        nodePath.toLocal8Bit(),                 //
        QCryptographicHash::Algorithm::Sha1);
    return hash.toHex();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(configFile(), QSettings::Format::NativeFormat)
    , m_macroPath(FUSION_MACRO_PATH)
    , m_iconPath(picturePath())
    , m_outputName(bundleOutputName())
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/assets/png/drfxbuilder.png"));
    setWindowTitle(qApp->applicationDisplayName());

    ui->statusbar->setSizeGripEnabled(true);
    ui->statusbar->showMessage(tr("Import Fusion macro file."), -1);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, ui->centralwidget);
    splitter->setSizes(QList<int>()                                      //
                       << m_settings.value("splitter.left", 160).toInt() //
                       << m_settings.value("splitter.right", 400).toInt());
    splitter->setHandleWidth(10);
    splitter->addWidget(ui->pnlBundle);
    splitter->addWidget(ui->pnlContent);
    ui->centralwidget->layout()->addWidget(splitter);
    connect(splitter, &QSplitter::splitterMoved, this, [this](int pos, int index) {
        switch (index) {
            case 0: {
                m_settings.setValue("splitter.left", pos);
                return;
            }
            case 1: {
                m_settings.setValue("splitter.right", pos);
                return;
            }
        }
    });

    connect(qApp, &QApplication::aboutToQuit, this, [this] {
        saveBundleStructure();
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

    m_outputName = m_settings.value("output.name", m_outputName).toString();
    m_macroPath = m_settings.value("macro.path", m_macroPath).toString();
    m_iconPath = m_settings.value("icon.path", m_iconPath).toString();

    // restore tree view first */
    loadBundleStructure();

    // async update ui fields
    checkBlackmagic();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_edFusionMacro_textEdited(const QString &value)
{
    const QFileInfo fi(value);
    m_macroPath = fi.absolutePath();
    m_settings.setValue("macro.file", value);
    m_settings.setValue("macro.path", m_macroPath);
    checkInputFields();
    updateTargetInfo();
}

void MainWindow::on_edIconFile_textEdited(const QString &value)
{
    const QFileInfo fi(value);
    m_iconPath = fi.absolutePath();
    m_settings.setValue("icon.file", value);
    m_settings.setValue("icon.path", m_iconPath);
    checkInputFields();
    updateTargetInfo();
}

void MainWindow::on_edCompany_textEdited(const QString &value)
{
    m_settings.setValue("company", value);
    checkInputFields();
    updateTargetInfo();
}

void MainWindow::on_edProduct_textEdited(const QString &value)
{
    m_settings.setValue("product", value);
    checkInputFields();
    updateTargetInfo();
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
    d.setDefaultSuffix(".setting");
    d.exec();
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

void MainWindow::on_tvBundleStruct_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    // sync combobox
    for (int i = 0; i < ui->cbBundleNode->count(); i++) {
        const QVariant v = ui->cbBundleNode->itemData(i, Qt::UserRole);
        if (v.isNull() || !v.isValid()) {
            continue;
        }
        QTreeWidgetItem *twi;
        if (!(twi = v.value<QTreeWidgetItem *>())) {
            continue;
        }
        if (current == twi) {
            ui->cbBundleNode->setCurrentIndex(i);
            break; // fini loop
        }
    }

    // fill table view or select or cleanup

    const QVariant title = current->data(0, Qt::DisplayRole);
    if (title.isNull() || !title.isValid()) {
        return;
    }
    const QVariant data = current->data(0, Qt::ItemDataRole::UserRole);
    if (data.isNull() || !data.isValid()) {
        return;
    }

    const TNodeData nd = data.value<TNodeData>();
    switch (nd.type) {
        // add to table view if product node
        case Product: {
            // reset table view
            while (ui->twNodeList->rowCount() > 0) {
                QTableWidgetItem *a1 = ui->twNodeList->takeItem(0, 0);
                QTableWidgetItem *a2 = ui->twNodeList->takeItem(0, 1);
                ui->twNodeList->removeRow(0);
                delete a1;
                delete a2;
            }
            // add items from tree view
            for (int i = 0; i < current->childCount(); i++) {
                QTreeWidgetItem *child = current->child(i);
                const QVariant data = child->data(0, Qt::ItemDataRole::UserRole);
                if (data.isNull() || !data.isValid()) {
                    continue;
                }
                const TNodeData nd = data.value<TNodeData>();
                if (nd.type != FileItem) {
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
            ui->pbDelete->setEnabled(false);
            break;
        }
        // select item in table view
        case FileItem: {
            for (int i = 0; i < ui->twNodeList->rowCount(); i++) {
                QTableWidgetItem *item;
                if ((item = ui->twNodeList->item(i, 1))) {
                    const QVariant data = item->data(Qt::ItemDataRole::UserRole);
                    if (data.isNull() || !data.isValid()) {
                        return;
                    }
                    if (data.value<QTreeWidgetItem *>() == current) {
                        ui->twNodeList->setCurrentItem(item);
                    }
                }
            }
            break;
        }
        // cleanup table
        default: {
            ui->pbDelete->setEnabled(false);
            while (ui->twNodeList->rowCount() > 0) {
                QTableWidgetItem *a1 = ui->twNodeList->takeItem(0, 0);
                QTableWidgetItem *a2 = ui->twNodeList->takeItem(0, 1);
                ui->twNodeList->removeRow(0);
                delete a1;
                delete a2;
            }
            break;
        }
    }
}

// select node in tree view
void MainWindow::on_twNodeList_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *)
{
    const QVariant data = current->data(Qt::ItemDataRole::UserRole);
    if (data.isNull() || !data.isValid()) {
        return;
    }
    const TNodeData nd = data.value<TNodeData>();
    if (nd.type != FileItem) {
        return;
    }
    QTreeWidgetItem *twi;
    if (!(twi = findName(ui->tvBundleStruct->topLevelItem(0), nd.name))) {
        ui->pbDelete->setEnabled(false);
        return;
    }

    ui->pbDelete->setEnabled(true);
    ui->tvBundleStruct->setCurrentItem(twi);
}

void MainWindow::on_pbImport_clicked()
{
    QString path = ui->cbBundleNode->currentText();
    QVariant v = ui->cbBundleNode->currentData(Qt::UserRole);
    if (v.isNull() || !v.isValid()) {
        return;
    }

    QFileInfo info;
    TNodeData data;
    QTreeWidgetItem *root, *twi, *child, *comp, *prod;
    int state = 0;

    if (!(root = v.value<QTreeWidgetItem *>())) {
        return;
    }
    if (root == ui->tvBundleStruct->topLevelItem(0)) {
        QMessageBox::critical(this,
                              qApp->applicationDisplayName(), //
                              tr("Items below 'Edit' node are not allowed."));
    }

    if (!(child = findCompanyAndProduct(root))) {
        // add company node
        path = path + "/" + ui->edCompany->text();
        data = {Company, toHash(path), path, ui->edCompany->text()};
        comp = new QTreeWidgetItem(root, QTreeWidgetItem::Type);
        comp->setData(0, Qt::DisplayRole, QVariant::fromValue(data.name));
        comp->setData(0, Qt::UserRole, QVariant::fromValue(data));
        root->addChild(comp);

        // add product node as child of company
        path = path + "/" + ui->edProduct->text();
        data = {Product, toHash(path), path, ui->edProduct->text()};
        prod = new QTreeWidgetItem(comp, QTreeWidgetItem::Type);
        prod->setData(0, Qt::DisplayRole, QVariant::fromValue(data.name));
        prod->setData(0, Qt::UserRole, QVariant::fromValue(data));
        comp->addChild(prod);
        twi = prod;
        // new company and product tree
        state = 0xf0;
    } else {
        twi = child;
    }

    // add macro file name
    info = QFileInfo(ui->edFusionMacro->text());
    if (findName(ui->tvBundleStruct->topLevelItem(0), info.baseName())) {
        QMessageBox::critical(this,
                              qApp->applicationDisplayName(), //
                              tr("Object '%1' already exist.").arg(info.baseName()));
        state |= 0x01;
    } else {
        data = {FileItem, toHash(info.filePath()), info.filePath(), info.baseName()};
        child = new QTreeWidgetItem(twi, QTreeWidgetItem::Type);
        child->setData(0, Qt::UserRole, QVariant::fromValue(data));
        child->setData(0, Qt::DisplayRole, data.name);
        twi->addChild(child);
    }

    // add icon file name
    info = QFileInfo(ui->edIconFile->text());
    if (findName(ui->tvBundleStruct->topLevelItem(0), info.baseName())) {
        QMessageBox::critical(this,
                              qApp->applicationDisplayName(), //
                              tr("Object '%1' already exist.").arg(info.baseName()));
        state |= 0x02;
    } else {
        data = {FileItem, toHash(info.filePath()), info.filePath(), info.baseName()};
        child = new QTreeWidgetItem(twi, QTreeWidgetItem::Type);
        child->setData(0, Qt::UserRole, QVariant::fromValue(data));
        child->setData(0, Qt::DisplayRole, data.name);
        twi->addChild(child);
    }

    if ((state & 0x01) && (state & 0x02) && (state & 0xf0)) {
        comp->removeChild(prod);
        delete prod;
        root->removeChild(comp);
        delete comp;
    }

    if (checkBundleContent(ui->tvBundleStruct->topLevelItem(0))) {
        ui->pbBuildDRFX->setEnabled(true);
        ui->pbBuildDRFX->setDefault(true);
        ui->pbImport->setDefault(false);
    }
}

// select node in tree view
void MainWindow::on_pbDelete_clicked()
{
    QTableWidgetItem *item = ui->twNodeList->currentItem();
    const QVariant title = item->data(Qt::DisplayRole);
    if (title.isNull() || !title.isValid()) {
        return;
    }
    const QVariant data = item->data(Qt::ItemDataRole::UserRole);
    if (data.isNull() || !data.isValid()) {
        return;
    }
    TNodeData nd = data.value<TNodeData>();
    if (nd.type != FileItem) {
        return;
    }
    if (QMessageBox::question(this,
                              qApp->applicationDisplayName(), //
                              tr("Do you want to delete object: %1").arg(title.toString()))) {
        //TODO: delete tree and table items
    }
}

void MainWindow::on_pbBuildDRFX_clicked()
{
    if (QMessageBox::question(this,
                              qApp->applicationDisplayName(), //
                              tr("Do you want to build DRFX bundle?\nBundle: %1").arg(m_outputName))) {
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

inline void MainWindow::postInitUi()
{
    ui->tvBundleStruct->expandAll();
    ui->edFusionMacro->setPlaceholderText(tr("Enter macro file name"));
    ui->edFusionMacro->setText(m_settings.value("macro.file", m_macroPath).toString());
    ui->edIconFile->setPlaceholderText(tr("Enter icon file name"));
    ui->edIconFile->setText(m_settings.value("icon.file", m_iconPath).toString());
    ui->edCompany->setPlaceholderText(tr("Enter company name"));
    ui->edCompany->setText(m_settings.value("company").toString());
    ui->edProduct->setPlaceholderText(tr("Enter product name"));
    ui->edProduct->setText(m_settings.value("product").toString());

    int nodesel = m_settings.value("nodesel", 0).toInt();
    if (ui->tvBundleStruct->topLevelItemCount() != 0) {
        cbxAddBundleItems(ui->tvBundleStruct->topLevelItem(0));
        if (ui->cbBundleNode->count() > nodesel) {
            ui->cbBundleNode->setCurrentIndex(nodesel);
            on_cbBundleNode_activated(nodesel);
        }
    }

    checkInputFields();

    if (checkBundleContent(ui->tvBundleStruct->topLevelItem(0))) {
        ui->pbImport->setDefault(false);
        ui->pbBuildDRFX->setEnabled(true);
        ui->pbBuildDRFX->setDefault(true);
    }
}

inline void MainWindow::updateTargetInfo()
{
    //QVariant v = ui->cbBundleNode->currentData(Qt::UserRole);
    //if (v.isNull() || !v.isValid()) {
    //    return;
    //}
    QString text = ui->cbBundleNode->currentText();
    text = text.append("/").append(ui->edCompany->text());
    text = text.append("/").append(ui->edProduct->text());
    ui->txTargetPath->setText(text);
}

inline void MainWindow::checkBlackmagic()
{
    // check which installed Fusion or Davinci or both
    if (m_macroPath.isEmpty()) {
        QTimer::singleShot(50, this, [this] {
            const QIcon icon(":/assets/png/dfrxbuilder.png");
            const QDir dbmf(FUSION_MACRO_PATH);
            const QDir dbmd(DAVINCI_MACRO_PATH);
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
                mb.setText(tr("Blackmagic Davinci Resolve and Fusion found.\n" //
                              "Please select which one to use."));
                mb.setIconPixmap(icon.pixmap(32, 32));
                mb.setStandardButtons(QMessageBox::Button::Ok |  //
                                      QMessageBox::Button::Yes | //
                                      QMessageBox::Button::Cancel);
                mb.setButtonText(QMessageBox::Button::Ok, tr("Davinci Resolve"));
                mb.setButtonText(QMessageBox::Button::Yes, tr("Fusion"));
                mb.setButtonText(QMessageBox::Button::Cancel, tr("Close"));
                mb.setDefaultButton(QMessageBox::Button::Yes);
                mb.setWindowTitle(qApp->applicationDisplayName());
                switch (mb.exec()) {
                    case QMessageBox::Button::Yes: {
                        m_macroPath = dbmf.absolutePath();
                        break;
                    }
                    case QMessageBox::Button::Ok: {
                        m_macroPath = dbmd.absolutePath();
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
            }
            // davinci
            else if ((flags & 0x02) == 0x02) {
                m_macroPath = dbmd.absolutePath();
            } else {
                QMessageBox::information(this,
                                         qApp->applicationDisplayName(), //
                                         tr("Blackmagic Davinci Resolve nor Fusion not found."));
            }
            m_settings.setValue("macro.path", m_macroPath);
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
            if (nd.type == Product) {
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

inline QTreeWidgetItem *MainWindow::findCompanyAndProduct(QTreeWidgetItem *node)
{
    QTreeWidgetItem *result = nullptr;
    for (int i = 0; i < node->childCount(); i++) {
        QTreeWidgetItem *child = node->child(i);
        QVariant v = child->data(0, Qt::DisplayRole);
        if (v.isNull() || !v.isValid()) {
            continue;
        }
        if (v.toString() == ui->edProduct->text()) {
            return child;
        }
        if (v.toString() == ui->edCompany->text()) {
            result = findCompanyAndProduct(child);
        }
    }
    return result;
}

inline QTreeWidgetItem *MainWindow::findName(QTreeWidgetItem *item, const QString &name)
{
    QTreeWidgetItem *result = nullptr;
    for (int i = 0; i < item->childCount(); i++) {
        QTreeWidgetItem *child = item->child(i);
        QVariant v = child->data(0, Qt::DisplayRole);
        if (v.isNull() || !v.isValid()) {
            continue;
        }
        if (v.toString() == name) {
            return child;
        }
        if (child->childCount() > 0) {
            result = findName(child, name);
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
    bundleStructToJson(root, ui->tvBundleStruct->topLevelItem(0), "");

    if (!root.isEmpty()) {
        const QJsonDocument jdoc(root);
        f.write(jdoc.toJson(QJsonDocument::Indented));
        //f.write(jdoc.toJson(QJsonDocument::Compact));
        f.flush();
        f.close();
    }
}

inline void MainWindow::bundleStructToJson(QJsonObject &node, QTreeWidgetItem *item, const QString &prefix)
{
    if (item->childCount() == 0) {
        return;
    }

    for (int i = 0; i < item->childCount(); i++) {
        QTreeWidgetItem *twi = item->child(i);

        QVariant title = twi->data(0, Qt::ItemDataRole::DisplayRole);
        if (title.isNull() || !title.isValid()) {
            continue;
        }

        QString s = title.toString();
        if (!prefix.isEmpty()) {
            s = prefix + "/" + title.toString();
        }

        QJsonObject jo;

        // additional data
        const QVariant data = twi->data(0, Qt::ItemDataRole::UserRole);
        if (!data.isNull() && data.isValid()) {
            TNodeData nd = data.value<TNodeData>();
            if (nd.hash.isEmpty()) {
                nd.hash = toHash(nd.name);
            }
            jo["tree.type"] = QJsonValue(nd.type);
            jo["data.hash"] = QJsonValue(nd.hash);
            jo["data.name"] = QJsonValue(nd.name);
            jo["data.path"] = QJsonValue(nd.path);
        }
        jo["tree.path"] = QJsonValue(s);

        // store to node object
        node[toHash(s)] = jo;

        // gather child nodes
        if (twi->childCount() > 0) {
            bundleStructToJson(node, twi, s);
        }
    }
}

inline bool MainWindow::loadBundleStructure()
{
    QFile f(bundleStuctureFile());
    if (!f.open(QFile::ReadOnly)) {
        return false; // may not exist (first run)
    }

    return true;
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
}
