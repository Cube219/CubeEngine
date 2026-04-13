# Environment mapping texture
# From Emil Persson, aka Humus (http://www.humus.name)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$TmpDir = Join-Path $ScriptDir ".tmp"

function Fetch-Resource {
    param(
        [string]$DstDir,
        [string]$Name,
        [string]$Url
    )

    $modelDir = Join-Path $DstDir $Name
    if (Test-Path $modelDir) {
        Write-Host "Skipping $Name (already exists)"
        return
    }

    Write-Host "Downloading $Name..."
    New-Item -ItemType Directory -Force -Path $TmpDir | Out-Null
    $zipPath = Join-Path $TmpDir "$Name.zip"
    Invoke-WebRequest -Uri $Url -OutFile $zipPath -UseBasicParsing

    Write-Host "Extracting $Name..."
    $extractDir = Join-Path $TmpDir "${Name}_extract"
    New-Item -ItemType Directory -Force -Path $extractDir | Out-Null
    Expand-Archive -Path $zipPath -DestinationPath $extractDir -Force

    New-Item -ItemType Directory -Force -Path $DstDir | Out-Null
    Move-Item -Path $extractDir -Destination $modelDir

    Remove-Item -Path $zipPath -Force -ErrorAction SilentlyContinue
    Remove-Item -Path $extractDir -Recurse -Force -ErrorAction SilentlyContinue

    Write-Host "$Name done."
}

Write-Host "Fetching IBL texture..."
$IBLPath = Join-Path $ScriptDir "Textures"
$IBLPath = Join-Path $IBLPath "IBL"
Fetch-Resource -DstDir $IBLPath -Name "NissiBeach2" -Url "https://www.humus.name/Textures/NissiBeach2.zip"

if (Test-Path $TmpDir) {
    Remove-Item -Path $TmpDir -Force -ErrorAction SilentlyContinue
}

Write-Host "Done."
