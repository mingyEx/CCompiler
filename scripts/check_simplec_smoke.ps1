param()

$repo = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repo 'Debug\SimpleC.exe'
$sampleDir = Join-Path $repo 'tests\simplec_smoke'

if (-not (Test-Path -LiteralPath $exe))
{
    Write-Host 'SimpleC.exe is missing. Build SimpleC Debug|Win32 first.' -ForegroundColor Red
    exit 1
}

if (-not (Test-Path -LiteralPath $sampleDir))
{
    Write-Host 'SimpleC smoke sample directory is missing.' -ForegroundColor Red
    exit 1
}

function Remove-SimpleCArtifacts
{
    param(
        [Parameter(Mandatory=$true)]
        [string] $SamplePath
    )

    $directory = Split-Path -Parent $SamplePath
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($SamplePath)
    $literalArtifacts = @(
        (Join-Path $directory ($baseName + '.code'))
    )

    foreach ($artifact in $literalArtifacts)
    {
        if (Test-Path -LiteralPath $artifact)
        {
            Remove-Item -LiteralPath $artifact -Force
        }
    }

    $patterns = @(
        ($baseName + '.asm.*.txt'),
        ($baseName + '.cfg.*.cfgdump')
    )
    foreach ($pattern in $patterns)
    {
        $matches = Get-ChildItem -LiteralPath $directory -Filter $pattern
        foreach ($match in $matches)
        {
            Remove-Item -LiteralPath $match.FullName -Force
        }
    }
}

$samples = Get-ChildItem -LiteralPath $sampleDir -Filter '*.txt'
if ($samples.Count -eq 0)
{
    Write-Host 'No SimpleC smoke samples found.' -ForegroundColor Red
    exit 1
}

foreach ($sample in $samples)
{
    Remove-SimpleCArtifacts -SamplePath $sample.FullName
    & $exe $sample.FullName
    if ($LASTEXITCODE -ne 0)
    {
        Write-Host ('SimpleC smoke failed: ' + $sample.Name) -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Remove-SimpleCArtifacts -SamplePath $sample.FullName
    Write-Host ('OK: SimpleC smoke passed: ' + $sample.Name) -ForegroundColor Green
}

exit 0
