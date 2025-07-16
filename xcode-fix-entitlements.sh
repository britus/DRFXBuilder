# Type a script or drag a script file from your workspace to insert its path.
# NOTE!!! -> Paste the script into Xcode Build Phase - custom build script
# Input files:
# ---> ${SOURCE_ROOT}/../../Entitlements.plist
# ---> ${SOURCE_ROOT}/../../Entitlements.plist
# Output files:
#  LEAVE EMPTY
echo "-------- PATCH WRONG ENTITLEMENTS ---------"
echo "-- build env"
printenv
##--
mkdir -p ${BUILD_ROOT}/DRFXBuilder.build/Debug/DRFXBuilder.build/DerivedSources
cp -pv ${SOURCE_ROOT}/../../Entitlements.plist ${BUILD_ROOT}/DRFXBuilder.build/Debug/DRFXBuilder.build/DerivedSources/Entitlements.plist
cp -pv ${SOURCE_ROOT}/../../DRFXBuilder.app.xcent ${BUILD_ROOT}/DRFXBuilder.build/Debug/DRFXBuilder.build/DRFXBuilder.app.xcent
##--
mkdir -p ${BUILD_ROOT}/DRFXBuilder.build/Release/DRFXBuilder.build/DerivedSources
cp -pv ${SOURCE_ROOT}/../../Entitlements.plist ${BUILD_ROOT}/DRFXBuilder.build/Release/DRFXBuilder.build/DerivedSources/Entitlements.plist
cp -pv ${SOURCE_ROOT}/../../DRFXBuilder.app.xcent ${BUILD_ROOT}/DRFXBuilder.build/Release/DRFXBuilder.build/DRFXBuilder.app.xcent
##--
mkdir -p ${TARGET_TEMP_DIR}/DerivedSources
cp -pv ${SOURCE_ROOT}/../../Entitlements.plist ${TARGET_TEMP_DIR}/DerivedSources/Entitlements.plist
cp -pv ${SOURCE_ROOT}/../../DRFXBuilder.app.xcent ${TARGET_TEMP_DIR}/DRFXBuilder.app.xcent
