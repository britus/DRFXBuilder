# Type a script or drag a script file from your workspace to insert its path.
# This scriptlet is only for Xcode custom build script in Build Phase
if [ "x${BUILT_PRODUCTS_DIR}" == "x" ] ; then
	echo "Paste the script into Xcode Build Phase - custom build script, or"
	echo "set QTDIR=<where your QT arch root>"
	echo "set PROJECT_ROOT=`pwd`"
	echo 'set BUILT_PRODUCTS_DIR=${PROJECT_ROOT}/build/xcode/<Debug|Release>'
	echo 'set PLUGINS_FOLDER_PATH=DRFXBuilder.app/Contents/PlugIns'
	exit 1
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
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/bearer && \
   cp -v ${QTDIR}/PlugIns/bearer/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/bearer
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/geometryloaders && \
   cp -v ${QTDIR}/PlugIns/geometryloaders/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/geometryloaders
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/printsupport && \
   cp -v ${QTDIR}/PlugIns/printsupport/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/printsupport
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/sceneparsers && \
   cp -v ${QTDIR}/PlugIns/sceneparsers/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/sceneparsers
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/texttospeech && \
   cp -v ${QTDIR}/PlugIns/texttospeech/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/texttospeech
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/texttospeech && \
   cp -v ${QTDIR}/PlugIns/texttospeech/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/texttospeech
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/virtualkeyboard && \
   cp -v ${QTDIR}/PlugIns/virtualkeyboard/*.dylib ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/virtualkeyboard
echo "--- Done ----"
