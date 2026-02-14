# Models downloaded from Morgan McGuire's Computer Graphics Archive https://casual-effects.com/data

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$DstDir = Join-Path $ScriptDir "DefaultModels"
$TmpDir = Join-Path $ScriptDir ".tmp"

function Fetch-Model {
    param(
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

Fetch-Model -Name "CornellBox" -Url "https://casual-effects.com/g3d/data10/common/model/CornellBox/CornellBox.zip"
Fetch-Model -Name "FireplaceRoom" -Url "https://casual-effects.com/g3d/data10/research/model/fireplace_room/fireplace_room.zip"
Fetch-Model -Name "LivingRoom" -Url "https://casual-effects.com/g3d/data10/research/model/living_room/living_room.zip"

if (Test-Path $TmpDir) {
    Remove-Item -Path $TmpDir -Force -ErrorAction SilentlyContinue
}

Write-Host "Done."
