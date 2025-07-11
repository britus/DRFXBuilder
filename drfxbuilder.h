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
    void build(QTreeWidgetItem *root);
    inline int errorCode() { return m_error; }
    inline bool isError() { return m_error != 0; }
    inline void setOutputName(const QString &name) { m_outputName = name; }

signals:
    void buildError(DRFXBuilder *builder, const QString &);
    void buildStarted(DRFXBuilder *builder);
    void buildComplete(DRFXBuilder *builder, const QString &);

private:
    int m_error;
    QThread *m_thread;
    QString m_outputName;

private:
    inline int createBundle(QThread *t, QZipWriter *zip, QTreeWidgetItem *node, const QString &prefix);
    inline void doStarted(QThread *t, QTreeWidgetItem *node);
    inline void doComplete();
};
