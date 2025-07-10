#!/bin/bash
DEVID=`cat .devid`
macdeployqt build/mediacleanup.app -always-overwrite -timestamp -appstore-compliant -codesign=${DEVID}
