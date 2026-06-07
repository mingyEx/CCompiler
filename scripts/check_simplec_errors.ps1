param()

$repo = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repo 'Debug\SimpleC.exe'
$sampleDir = Join-Path $repo 'tests\simplec_errors'

if (-not (Test-Path -LiteralPath $exe))
{
    Write-Host 'SimpleC.exe is missing. Build SimpleC Debug|Win32 first.' -ForegroundColor Red
    exit 1
}

if (-not (Test-Path -LiteralPath $sampleDir))
{
    Write-Host 'SimpleC error sample directory is missing.' -ForegroundColor Red
    exit 1
}

$samples = Get-ChildItem -LiteralPath $sampleDir -Filter '*.txt'
if ($samples.Count -eq 0)
{
    Write-Host 'No SimpleC error samples found.' -ForegroundColor Red
    exit 1
}

foreach ($sample in $samples)
{
    & $exe $sample.FullName
    if ($LASTEXITCODE -eq 0)
    {
        Write-Host ('SimpleC error sample unexpectedly passed: ' + $sample.Name) -ForegroundColor Red
        exit 1
    }
    Write-Host ('OK: SimpleC error sample rejected: ' + $sample.Name) -ForegroundColor Green
}

exit 0
