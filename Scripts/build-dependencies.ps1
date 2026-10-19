<#
.LICENSE
Copyright (c) 2026 DLR e.V.

This program and the accompanying materials are made
available under the terms of the Eclipse Public License 2.0
which is available at https://www.eclipse.org/legal/epl-2.0/

SPDX-License-Identifier: EPL-2.0
#>

param(
    [string]$ProtobufVersion = "21.12",
    [string]$OsiVersion = "v3.8.0",
    [string]$Configuration = "Release",
    [string]$Architecture = "x64",
    [string]$Generator = "Visual Studio 17 2022",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Resolve-Tool {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $tool = Get-Command $Name -ErrorAction SilentlyContinue
    if (-not $tool) {
        throw "Required tool '$Name' was not found in PATH."
    }

    return $tool.Source
}

function Invoke-Native {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [string[]]$Arguments
    )

    Write-Host "> $FilePath $($Arguments -join ' ')"
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $FilePath"
    }
}

function Copy-DirectoryContents {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,
        [Parameter(Mandatory = $true)]
        [string]$Destination
    )

    if (-not (Test-Path $Source)) {
        throw "Source directory does not exist: $Source"
    }

    New-Item -ItemType Directory -Force -Path $Destination | Out-Null
    Copy-Item -Path (Join-Path $Source "*") -Destination $Destination -Recurse -Force
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir = Resolve-Path (Join-Path $scriptDir "..")
$thirdPartyDir = Join-Path $rootDir "ThirdParty"
$workDir = Join-Path $thirdPartyDir ".build/windows"
$downloadsDir = Join-Path $workDir "downloads"
$installDir = Join-Path $workDir "install"
$protobufInstallDir = Join-Path $installDir "protobuf"
$protobufSourceDir = Join-Path $workDir "protobuf-$ProtobufVersion"
$osiSourceDir = Join-Path $workDir "osi-cpp"

$protobufUrl = "https://github.com/protocolbuffers/protobuf/releases/download/v$ProtobufVersion/protobuf-all-$ProtobufVersion.zip"
$protobufArchive = Join-Path $downloadsDir "protobuf-all-$ProtobufVersion.zip"

$protobufTargetIncludeDir = Join-Path $thirdPartyDir "protobuf/include"
$protobufTargetLibDir = Join-Path $thirdPartyDir "protobuf/lib"
$osiTargetIncludeDir = Join-Path $thirdPartyDir "osi3/include/osi3"
$osiTargetLibDir = Join-Path $thirdPartyDir "osi3/lib/osi3"

Resolve-Tool "git" | Out-Null
Resolve-Tool "cmake" | Out-Null

if ($Clean -and (Test-Path $workDir)) {
    Remove-Item -Recurse -Force $workDir
}

New-Item -ItemType Directory -Force -Path $downloadsDir, $protobufTargetIncludeDir, $protobufTargetLibDir, $osiTargetIncludeDir, $osiTargetLibDir | Out-Null

if (-not (Test-Path $protobufArchive)) {
    Write-Host "Downloading Protobuf $ProtobufVersion..."
    Invoke-WebRequest -Uri $protobufUrl -OutFile $protobufArchive
}

if (-not (Test-Path $protobufSourceDir)) {
    Write-Host "Extracting Protobuf $ProtobufVersion..."
    Expand-Archive -Path $protobufArchive -DestinationPath $workDir -Force
}

$protobufBuildDir = Join-Path $workDir "protobuf-build"
Invoke-Native -FilePath "cmake" -Arguments @(
    "-S", (Join-Path $protobufSourceDir "cmake"),
    "-B", $protobufBuildDir,
    "-G", $Generator,
    "-A", $Architecture,
    "-DCMAKE_INSTALL_PREFIX=$protobufInstallDir",
    "-DCMAKE_BUILD_TYPE=$Configuration",
    "-Dprotobuf_BUILD_TESTS=OFF",
    "-Dprotobuf_BUILD_EXAMPLES=OFF",
    "-Dprotobuf_BUILD_CONFORMANCE=OFF",
    "-Dprotobuf_BUILD_SHARED_LIBS=OFF",
    "-Dprotobuf_MSVC_STATIC_RUNTIME=OFF"
)
Invoke-Native -FilePath "cmake" -Arguments @("--build", $protobufBuildDir, "--config", $Configuration, "--parallel")
Invoke-Native -FilePath "cmake" -Arguments @("--install", $protobufBuildDir, "--config", $Configuration)

if (-not (Test-Path $osiSourceDir)) {
    Invoke-Native -FilePath "git" -Arguments @("clone", "https://github.com/OpenSimulationInterface/osi-cpp.git", $osiSourceDir)
}

Invoke-Native -FilePath "git" -Arguments @("-C", $osiSourceDir, "fetch", "--tags", "--force")
Invoke-Native -FilePath "git" -Arguments @("-C", $osiSourceDir, "checkout", $OsiVersion)
Invoke-Native -FilePath "git" -Arguments @("-C", $osiSourceDir, "submodule", "update", "--init", "--recursive")

$protobufLib = Join-Path $protobufInstallDir "lib/libprotobuf.lib"
$protocExe = Join-Path $protobufInstallDir "bin/protoc.exe"
$protobufIncludeDir = Join-Path $protobufInstallDir "include"

foreach ($requiredPath in @($protobufLib, $protocExe, $protobufIncludeDir)) {
    if (-not (Test-Path $requiredPath)) {
        throw "Expected Protobuf artefact was not created: $requiredPath"
    }
}

$osiBuildDir = Join-Path $workDir "osi-cpp-build"
Invoke-Native -FilePath "cmake" -Arguments @(
    "-S", $osiSourceDir,
    "-B", $osiBuildDir,
    "-G", $Generator,
    "-A", $Architecture,
    "-DCMAKE_BUILD_TYPE=$Configuration",
    "-DProtobuf_USE_STATIC_LIBS:BOOL=ON",
    "-DProtobuf_INCLUDE_DIR=$protobufIncludeDir",
    "-DProtobuf_LIBRARY=$protobufLib",
    "-DProtobuf_PROTOC_EXECUTABLE=$protocExe"
)
Invoke-Native -FilePath "cmake" -Arguments @("--build", $osiBuildDir, "--config", $Configuration, "--parallel")

$osiLib = Get-ChildItem -Path $osiBuildDir -Recurse -Filter "open_simulation_interface_static.lib" |
    Select-Object -First 1
if (-not $osiLib) {
    throw "Expected OSI library was not created under: $osiBuildDir"
}

$osiGeneratedHeader = Get-ChildItem -Path $osiBuildDir -Recurse -Filter "osi_version.pb.h" |
    Where-Object { $_.Directory.Name -eq "osi3" } |
    Select-Object -First 1
if (-not $osiGeneratedHeader) {
    throw "Expected OSI generated headers were not created under: $osiBuildDir"
}
$osiGeneratedIncludeDir = $osiGeneratedHeader.Directory.FullName

Copy-DirectoryContents -Source $protobufIncludeDir -Destination $protobufTargetIncludeDir
Copy-Item -Path $protobufLib -Destination (Join-Path $protobufTargetLibDir "libprotobuf.lib") -Force
Copy-DirectoryContents -Source $osiGeneratedIncludeDir -Destination $osiTargetIncludeDir
Copy-Item -Path $osiLib.FullName -Destination (Join-Path $osiTargetLibDir "open_simulation_interface_static.lib") -Force

Write-Host "Built Protobuf $ProtobufVersion and OSI $OsiVersion for Win64."
Write-Host "Refreshed:"
Write-Host "  $protobufTargetIncludeDir"
Write-Host "  $(Join-Path $protobufTargetLibDir "libprotobuf.lib")"
Write-Host "  $osiTargetIncludeDir"
Write-Host "  $(Join-Path $osiTargetLibDir "open_simulation_interface_static.lib")"
