## EoF DRFX Bundle Builder

### Overview

The project contains the user interface to create VFX bundle files for Blackmagics Davinci Resolve or Fusion.

### Dependencies

- QT framework and its dependencies.

```
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
mkdir -p ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}
cp -vR ${QTDIR}/PlugIns/* ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/
rm -fR ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/permissions
rm -fR ${BUILT_PRODUCTS_DIR}/${PLUGINS_FOLDER_PATH}/*/*.dSYM
```
