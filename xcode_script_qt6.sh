# Type a script or drag a script file from your workspace to insert its path.
# This scriptlet is only for Xcode custom build script in Build Phase
if [ "x${BUILT_PRODUCTS_DIR}" == "x" ] ; then
    echo "Paste the script into Xcode Build Phase - custom build script, or"
    echo "set QTDIR=<where your QT arch root>"
    echo "set PROJECT_ROOT=`pwd`"
    echo 'set BUILT_PRODUCTS_DIR=${PROJECT_ROOT}/build/xcode/<Debug|Release>'
    echo 'set PLUGINS_FOLDER_PATH=DRFXBuilder.app/Contents/PlugIns'
    exit 1
fi
echo "--- INSTALL QT-PLUGINS ---"
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/platforms && \
   cp -v ${QTDIR}/PlugIns/platforms/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/platforms
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/iconengines && \
   cp -v ${QTDIR}/PlugIns/iconengines/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/iconengines
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/imageformats && \
   cp -v ${QTDIR}/PlugIns/imageformats/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/imageformats
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/platforminputcontexts && \
   cp -v ${QTDIR}/PlugIns/platforminputcontexts/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/platforminputcontexts
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/texttospeech && \
   cp -v ${QTDIR}/PlugIns/texttospeech/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/texttospeech
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/styles && \
   cp -v ${QTDIR}/PlugIns/styles/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/styles
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/renderplugins && \
   cp -v ${QTDIR}/PlugIns/renderplugins/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/renderplugins
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/renderers && \
   cp -v ${QTDIR}/PlugIns/renderers/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/renderers
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/generic && \
   cp -v ${QTDIR}/PlugIns/generic/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/generic
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/tls && \
   cp -v ${QTDIR}/PlugIns/tls/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/tls
echo "--- INSTALL QT-FRAMEWORKS ---"
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtCore.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtDBus.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtGui.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtNetwork.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtOpenGL.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtPdf.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtQml.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtQmlMeta.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtQmlModels.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtQmlWorkerScript.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtQuick.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtSvg.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtVirtualKeyboard.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtQmlMeta.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtVirtualKeyboardQml.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/
mkdir -p ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH} && \
   cp -Rv ${QTDIR}/lib/QtWidgets.framework ${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/

echo "--- Done ----"
