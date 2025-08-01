#include "drfxbuilder.h"
#include "drfxtypes.h"
#ifdef Q_OS_MACOS
#include "drfxsandbox.h"
#endif
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

#ifdef QT_ZIP_MODULE
#include <QZipWriter>
#else
#include <qzipwriter.h>
#endif

#if 0
inline static QString tempPath()
{
    return QStandardPaths::writableLocation( //
        QStandardPaths::StandardLocation::TempLocation);
}
#endif

DRFXBuilder::DRFXBuilder(const QString &ouputDir, QObject *parent)
    : QObject(parent)
    , m_outputName(ouputDir)
    , itemCount(0)
{
    //
}

DRFXBuilder::~DRFXBuilder()
{
    //
}

void DRFXBuilder::build(QThread *t, QTreeWidgetItem *node)
{
    emit buildStarted(this);

    QZipWriter zip(QDir::toNativeSeparators(m_outputName));
    if (!zip.isWritable()) {
        emit buildError(this, tr("Unable to create DRFX bundle: %1").arg(m_outputName));
        m_error = -EIO;
        return;
    }
    if ((m_error = createBundle(t, &zip, node, "/"))) {
        QFile::remove(m_outputName);
        zip.close();
        return;
    }
    zip.close();

    // notify completion
    complete();

    t->exit(m_error);
}

inline void DRFXBuilder::complete()
{
    emit buildComplete(this, m_outputName);
}

inline int DRFXBuilder::createBundle(QThread *t, QZipWriter *zip, QTreeWidgetItem *node, const QString &prefix)
{
    int rc = EXIT_SUCCESS;
    QVariant v;

    if (t->isInterruptionRequested()) {
        return -ELOOP;
    }

    t->yieldCurrentThread();

    // node entry name
    v = node->data(0, Qt::DisplayRole);
    if (v.isNull() || !v.isValid()) {
        return -EINVAL;
    }

    // node data object
    v = node->data(0, Qt::UserRole);
    if (v.isNull() || !v.isValid()) {
        return -EINVAL;
    }

    TNodeData nd = v.value<TNodeData>();
    if (nd.type == TNodeType::NTNone) {
        return -EINVAL;
    }

    switch (nd.type) {
        case TNodeType::NTStatic:
        case TNodeType::NTCompany:
        case TNodeType::NTProduct: {
            const QFile::Permissions permissions = QFile::Permissions( //
                QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |  //
                QFile::ReadGroup | QFile::ExeGroup |                   //
                QFile::ReadOther | QFile::ExeOther);
            zip->setCompressionPolicy(QZipWriter::AutoCompress);
            zip->setCreationPermissions(permissions);
            zip->addDirectory(nd.path);
            if (zip->status() != QZipWriter::NoError) {
                emit buildError(this, tr("Unable to add zip-directory entry: %1").arg(nd.path));
                return -EIO;
            }
            itemCount++;
            emit buildItemsDone(itemCount);
            break;
        }
        case TNodeType::NTFileItem: {
            QFileInfo srcFi(nd.path);
            QString zipName = srcFi.baseName();
            if (!srcFi.suffix().isEmpty()) {
                zipName += "." + srcFi.suffix();
            }
#ifdef Q_OS_MACOS__
            void* secUrl;
            secUrl = openFileBookmark(QStringLiteral( //
                "file://%1").arg(nd.path).toNSString());
#endif
            QFile inFile(nd.path);
            if (!inFile.open(QIODevice::ReadOnly)) {
                emit buildError(this, tr("Unable to open source file: %1").arg(inFile.fileName()));
                return -EIO;
            }
            const QFile::Permissions permissions = QFile::Permissions( //
                QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther);
            zip->setCompressionPolicy(QZipWriter::AutoCompress);
            zip->setCreationPermissions(permissions);
            zip->addFile(prefix + "/" + zipName, &inFile);
            
#ifdef Q_OS_MACOS__
            if (secUrl) {
                closeFileBookmark(secUrl);
            }
#endif
            if (zip->status() != QZipWriter::NoError) {
                emit buildError(this, tr("Unable to add file entry: %1").arg(zipName));
                return -EIO;
            }
            itemCount++;
            emit buildItemsDone(itemCount);
            break;
        }
        default: {
            break;
        }
    }

    for (int i = 0; i < node->childCount(); i++) {
        if ((rc = createBundle(t, zip, node->child(i), nd.path))) {
            break;
        }
    }

    return rc;
}
