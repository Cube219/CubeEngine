@echo off
setlocal enabledelayedexpansion

goto main

:download_and_extract
set "NAME=%~1"
set "URL=%~2"

set TMP_PATH=.tmp\%NAME%.zip
set DST_PATH=DefaultModels\%NAME%

if not exist .tmp (
    mkdir .tmp
)

if exist %DST_PATH% (
    echo %NAME% already exists. Skip the downloading.
    exit /b 0
)

echo Downloading "%URL%" ...
powershell -Command "Invoke-WebRequest '%URL%' -OutFile '%TMP_PATH%'"
if errorlevel 1 (
    echo Download failed!
    exit /b 1
)

echo Extracting "%TMP_PATH%" to "%DST_PATH%" ...
powershell -Command "Expand-Archive -LiteralPath '%TMP_PATH%' -DestinationPath '%DST_PATH%' -Force"
if errorlevel 1 (
    echo Extraction failed!
    exit /b 1
)
del "%TMP_PATH%"

exit /b 0

:main
echo Models downloaded from Morgan McGuire's Computer Graphics Archive https://casual-effects.com/data

call :download_and_extract "StanfordBunny" "https://casual-effects.com/g3d/data10/research/model/bunny/bunny.zip"
call :download_and_extract "CornellBox" "https://casual-effects.com/g3d/data10/common/model/CornellBox/CornellBox.zip"
call :download_and_extract "FireplaceRoom" "https://casual-effects.com/g3d/data10/research/model/fireplace_room/fireplace_room.zip"
call :download_and_extract "LivingRoom" "https://casual-effects.com/g3d/data10/research/model/living_room/living_room.zip"

echo Done.
pause
