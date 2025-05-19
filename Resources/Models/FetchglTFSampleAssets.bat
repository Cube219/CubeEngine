@echo off

set REPO_URL=https://github.com/KhronosGroup/glTF-Sample-Assets.git
set DST_DIR=glTFSampleAssets

echo Fetching/Updating %DST_DIR%...

if exist "%DST_DIR%\.git" (
    pushd "%DST_DIR%"
    git pull --depth=1
    popd
) else (
    echo Cloning repository...
    git clone --depth=1 %REPO_URL% %DST_DIR%
)

echo Done.
pause
