$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$libDir = Join-Path $root 'lib'
$commonLibDir = Join-Path $libDir 'commonlibf4'

if (Test-Path $commonLibDir) {
    Write-Host "CommonLibF4 already exists at $commonLibDir"
    exit 0
}

New-Item -ItemType Directory -Force -Path $libDir | Out-Null
Write-Host 'Cloning libxse/commonlibf4 and its submodules...'
git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/libxse/commonlibf4 $commonLibDir
Write-Host 'CommonLibF4 ready.'
