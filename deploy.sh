#!/bin/bash
DEVID=`cat .devid`

macdeployqt build/DRFXBuilder.app -always-overwrite -timestamp -appstore-compliant -codesign=${DEVID}
