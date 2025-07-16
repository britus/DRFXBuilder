#pragma once
#include <QObject>
#include <QString>

#ifdef Q_OS_MACOS
#define FUSION_MACRO_PATH QStringLiteral("Library/Application Support/Blackmagic Design/Fusion/Macros")
#define FUSION_TEMPLATE_PATH QStringLiteral("Library/Application Support/Blackmagic Design/Fusion/Templates")
#define FUSION_SCRIPT_PATH QStringLiteral("Library/Application Support/Blackmagic Design/Fusion/Scripts/Comp")

#define DAVINCI_MACRO_PATH QStringLiteral("Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Macros")
#define DAVINCI_TEMPLATE_PATH QStringLiteral("Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Templates")
#define DAVINCI_SCRIPT_PATH QStringLiteral("Library/Application Support/Blackmagic Design/DaVinci Resolve/Fusion/Scripts/Comp")
#endif

#ifdef Q_OS_WINDOWS
#define FUSION_MACRO_PATH "AppData/Roaming/Blackmagic Design/Fusion/Macros"
#define FUSION_TEMPLATE_PATH "AppData/Roaming/Blackmagic Design/Fusion/Templates"
#define FUSION_SCRIPT_PATH "AppData/Roaming/Blackmagic Design/Fusion/Scripts/Comp"

#define DAVINCI_MACRO_PATH "AppData/Roaming/Blackmagic Design/DaVinci Resolve/Support/Fusion/Macros"
#define DAVINCI_TEMPLATE_PATH "AppData/Roaming/Blackmagic Design/DaVinci Resolve/Support/Fusion/Templates"
#define DAVINCI_SCRIPT_PATH "AppData/Roaming/Blackmagic Design/DaVinci Resolve/Support/Fusion/Scripts/Comp"
#endif

#ifdef Q_OS_LINUX
#define FUSION_MACRO_PATH ".local/share/Fusion/Macros"
#define FUSION_TEMPLATE_PATH ".local/share/Fusion/Templates"
#define FUSION_SCRIPT_PATH ".local/share/Fusion/Scripts/Comp"

#define DAVINCI_MACRO_PATH ".local/share/DaVinciResolve/Fusion/Macros"
#define DAVINCI_TEMPLATE_PATH ".local/share/DaVinciResolve/Fusion/Templates"
#define DAVINCI_SCRIPT_PATH ".local/share/DaVinciResolve/Fusion/Scripts/Comp"
#endif

#ifndef APP_TITLE
#define APP_TITLE QStringLiteral("EoF VFX Bundle Builder (.drfx)")
#endif

enum TAppType {
    ATNone = 0x00,
    ATDavinci = 0x01,
    ATFusion = 0x02,
    ATBoth = (ATDavinci | ATFusion),
};

enum TNodeType {
    NTNone,
    NTStatic,
    NTCompany,
    NTProduct,
    NTFileItem,
};

typedef struct node_data
{
    TNodeType type = TNodeType::NTNone;
    QString hash = "";
    QString path = "";
    QString name = "";
} TNodeData;

Q_DECLARE_METATYPE(TNodeType);
Q_DECLARE_METATYPE(TNodeData);
