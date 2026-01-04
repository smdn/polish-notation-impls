# SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT
# cSpell:ignore BUILDTOOLS
# cSpell:ignore MSBuilsSearch_impl
#
# This script is based on 'MSBuilsSearch_impl.ps1' from https://github.com/rot-z/MSBuildSearch, published under the MIT License.
#

# folder paths of Visual Studio
$MSBUILD_VS2022_BUILDTOOLS_PATH = "Microsoft Visual Studio`\2022`\BuildTools`\MSBuild`\Current`\Bin"
$MSBUILD_VS2019_BUILDTOOLS_PATH = "Microsoft Visual Studio`\2019`\BuildTools`\MSBuild`\Current`\Bin"
$MSBUILD_17_ENTERPRISE_PATH     = "Microsoft Visual Studio`\2022`\Enterprise`\MSBuild`\Current`\Bin"    # Visual Studio 2022 Enterprise
$MSBUILD_16_COMMUNITY_PATH      = "Microsoft Visual Studio`\2019`\Community`\MSBuild`\Current`\Bin"     # Visual Studio 2019 Community (not tested)
$MSBUILD_16_PROFESSIONAL_PATH   = "Microsoft Visual Studio`\2019`\Professional`\MSBuild`\Current`\Bin"  # Visual Studio 2019 Professional
$MSBUILD_15_COMMUNITY_PATH      = "Microsoft Visual Studio`\2017`\Community`\MSBuild`\15.0`\Bin"        # Visual Studio 2017 Community
$MSBUILD_15_PROFESSIONAL_PATH   = "Microsoft Visual Studio`\2017`\Professional`\MSBuild`\15.0`\Bin"     # Visual Studio 2017 Professional

# target paths for MSBuild
# sort by priority
[array]$SEARCH_PATHS = @(
    $MSBUILD_VS2022_BUILDTOOLS_PATH,
    $MSBUILD_VS2019_BUILDTOOLS_PATH,
    $MSBUILD_17_ENTERPRISE_PATH,
    $MSBUILD_16_COMMUNITY_PATH,
    $MSBUILD_16_PROFESSIONAL_PATH,
    $MSBUILD_15_COMMUNITY_PATH,
    $MSBUILD_15_PROFESSIONAL_PATH
)

# get full path of "Program Files" folder from OS architecture
$programFilesDir = ""
if ($([System.Runtime.InteropServices.RuntimeInformation]::ProcessArchitecture) -eq 'X64')
{
    $programFilesDir = ${env:ProgramFiles(x86)}
}
else
{
    $programFilesDir = ${env:ProgramFiles}
}

# search MSBuild.exe
$msbuildPath = ""
foreach($p in $SEARCH_PATHS)
{
    # is folder exists?
    $targetPath = Join-Path $programFilesDir $p
    if (!(Test-Path $targetPath))
    {
        continue
    }

    # select the most shortest (shallowest) path
    $results = (Get-ChildItem $targetPath -Include MSBuild.exe -Recurse).FullName | Sort-Object -Property Length
    if ($results.Length -gt 0)
    {
        $msbuildPath = $results[0]
        Write-Output $msbuildPath
        exit 0
    }
}

# not found
exit 1
