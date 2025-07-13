#pragma once

#include "drfxbuilder.h"
#include <QComboBox>
#include <QJsonObject>
#include <QMainWindow>
#include <QSettings>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setMacroPath(const QString &path, const QString &fileName = "");
    void setIconPath(const QString &path, const QString &fileName = "");
    void setOutputName(const QString &fileName);
    void setInstallPath(const QString &path);
    void setScriptPath(const QString &path);
    void setAppType(quint8 type);

private slots:
    void onBuildError(DRFXBuilder *builder, const QString &);
    void onBuildStarted(DRFXBuilder *builder);
    void onBuildComplete(DRFXBuilder *builder, const QString &);
    void on_tvBundleStruct_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_twNodeList_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_edFusionMacro_textEdited(const QString &arg1);
    void on_edIconFile_textEdited(const QString &arg1);
    void on_edCompany_textEdited(const QString &arg1);
    void on_edProduct_textEdited(const QString &arg1);
    void on_pbSelectMacro_clicked();
    void on_pbSelectIcon_clicked();
    void on_cbBundleNode_activated(int index);
    void on_pbImport_clicked();
    void on_pbDelete_clicked();
    void on_pbBuildDRFX_clicked();
    void on_pbInstall_clicked();
    void on_pbNewBundle_clicked();
    void on_edFusionMacro_textChanged(const QString &arg1);
    void on_edIconFile_textChanged(const QString &arg1);
    void on_edCompany_textChanged(const QString &arg1);
    void on_edProduct_textChanged(const QString &arg1);
    void on_twNodeList_itemClicked(QTableWidgetItem *item);

    void on_cbBundleNode_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QSettings m_settings;
    QString m_macroPath;
    QString m_iconPath;
    QString m_outputName;
    QString m_installPath;
    QString m_scriptPath;
    quint8 m_appType;

private:
    inline void postInitUi();
    inline void updateTargetInfo();
    inline void checkInputFields();
    inline void checkBlackmagic();
    inline void checkOutputExist();
    inline void addToNode(const QString &path, QTreeWidgetItem *root);
    inline QTreeWidgetItem *addCompanyNode(QTreeWidgetItem *node, const QString &name, const QString &path);
    inline QTreeWidgetItem *addProductNode(QTreeWidgetItem *node, const QString &name, const QString &path);
    inline int addFileNode(QTreeWidgetItem *node, int errorBits, const QString &fileName);
    inline bool checkBundleContent(QTreeWidgetItem *node);
    inline void cbxAddBundleItems(QTreeWidgetItem *node, const QString &prefix = "");
    inline QTreeWidgetItem *findNodeByHash(QTreeWidgetItem *node, const QString &hash);
    inline void bundleToJson(QJsonObject &node, QTreeWidgetItem *item);
    inline void saveBundleStructure();
    inline bool loadBundleStructure();
    inline void selectComboBoxItem(QTreeWidgetItem *node);
    inline void resetBundleStructure(QTreeWidgetItem *node);
    inline void selectTableRow(QTreeWidgetItem *node);
    inline void fillTableView(QTreeWidgetItem *node);
    inline void cleanupTableView();

    inline QString configPath() const;
    inline QString appDataPath() const;
    inline QString appLocalDataPath() const;
    inline QString configFile() const;
    inline QString bundleStuctureFile() const;
    inline QString picturePath() const;
    inline QString bundleOutputName() const;
    inline QString homePath() const;
    inline QString documentsPath() const;
    inline QString scriptPath(quint8 appType) const;
    inline QString templatePath(quint8 appType) const;
    inline QString macroPath(quint8 appType) const;
    inline QString toHash(const QString &nodePath) const;
    inline QString shortenText(const QString &value, int max) const;
};
