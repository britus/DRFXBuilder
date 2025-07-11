#pragma once
#include <QObject>
#include <QString>

enum TNodeType {
    None,
    Static,
    Company,
    Product,
    FileItem,
};

typedef struct
{
    TNodeType type;
    QString hash;
    QString path;
    QString name;
} TNodeData;

Q_DECLARE_METATYPE(TNodeType);
Q_DECLARE_METATYPE(TNodeData);
