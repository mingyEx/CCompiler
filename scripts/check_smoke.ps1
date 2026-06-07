param()

$repo = Split-Path -Parent $PSScriptRoot
$msbuild = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe'
$solution = Join-Path $repo 'CCompiler.sln'
$x86GeneratorProject = Join-Path $repo 'DevTools\X86_InstrCodeGen\X86_InstrCodeGen.vcxproj'
$noCoreLibGuard = Join-Path $repo 'scripts\check_mainchain_no_corelib.ps1'
$simpleCSmoke = Join-Path $repo 'scripts\check_simplec_smoke.ps1'
$simpleCErrors = Join-Path $repo 'scripts\check_simplec_errors.ps1'
$x86GeneratorDrift = Join-Path $repo 'scripts\check_x86_generator_drift.ps1'

if (-not (Test-Path -LiteralPath $msbuild))
{
    Write-Host 'MSBuild was not found at the expected Visual Studio 2022 Community path.' -ForegroundColor Red
    exit 1
}

& $msbuild $solution /t:SimpleC /p:Configuration=Debug /p:Platform=Win32 /m /nologo
if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

& $noCoreLibGuard
if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

& $simpleCSmoke
if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

& $simpleCErrors
if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

& $msbuild $x86GeneratorProject /p:Configuration=Debug /p:Platform=Win32 /m /nologo
if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

& $x86GeneratorDrift
if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

Write-Host 'OK: full smoke validation passed.' -ForegroundColor Green
exit 0
