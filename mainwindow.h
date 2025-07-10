#pragma once

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
    enum TNodeType {
        Static,
        Company,
        Product,
        FileItem,
    };
    Q_ENUM(TNodeType)

    typedef struct
    {
        TNodeType type;
        QString hash;
        QString path;
        QString name;
    } TNodeData;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
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

private:
    Ui::MainWindow *ui;
    QSettings m_settings;
    QString m_macroPath;
    QString m_iconPath;

private:
    inline void postInitUi();
    inline void updateTargetInfo();
    inline void checkInputFields();
    inline void checkBlackmagic();
    inline void cbxAddBundleItems(QTreeWidgetItem *item, const QString &prefix = "");
    inline QTreeWidgetItem *findCompanyAndProduct(QTreeWidgetItem *item);
    inline QTreeWidgetItem *findName(QTreeWidgetItem *item, const QString &name);
    inline void bundleStructToJson(QJsonObject &node, QTreeWidgetItem *item, const QString &prefix);
    inline void saveBundleStructure();
    inline bool loadBundleStructure();
};

Q_DECLARE_METATYPE(MainWindow::TNodeType);
Q_DECLARE_METATYPE(MainWindow::TNodeData);
