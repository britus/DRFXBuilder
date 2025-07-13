#pragma once
#include <QDir>
#include <QObject>
#include <QThread>
#include <QTreeWidgetItem>

#ifdef QT_COMPRESS_MODULE
#include <QZipWriter>
#else
#include <qzipwriter.h>
#endif

class DRFXBuilder : public QObject
{
    Q_OBJECT

public:
    explicit DRFXBuilder(const QString &outputName, QObject *parent = nullptr);
    ~DRFXBuilder();
    void build(QThread *t, QTreeWidgetItem *root);
    void complete();
    inline int errorCode() { return m_error; }
    inline bool isError() { return m_error != 0; }
    inline void setOutputName(const QString &name) { m_outputName = name; }
    inline const QString &outputName() const { return m_outputName; }

signals:
    void buildError(DRFXBuilder *builder, const QString &);
    void buildStarted(DRFXBuilder *builder);
    void buildComplete(DRFXBuilder *builder, const QString &);
    void buildItemsDone(int count);

private:
    int m_error;
    QString m_outputName;
    int itemCount;

private:
    inline int createBundle(QThread *t, QZipWriter *zip, QTreeWidgetItem *node, const QString &prefix);
};
