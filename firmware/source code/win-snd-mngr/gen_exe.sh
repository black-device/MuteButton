#WINPYTHONPATH=users/bd/AppData/Local/Programs/Python/Python310
WINPYTHONPATH=python/Python312
APPNAME=mute-btn-systray
APPVER=$(grep APP_VERSION $APPNAME.py | cut -d "'" -f 2 | head -n 1)

echo "Current app version: ${APPVER}"

wine ~/.wine/drive_c/${WINPYTHONPATH}/Scripts/pyinstaller.exe ${APPNAME}.spec

mv dist/${APPNAME}.exe dist/${APPNAME}_${APPVER}.exe
