param()

$repo = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $repo

$patterns = @(
    'CoreLib::',
    'CoreLib\\',
    'Basic\.h',
    '\bRefPtr<',
    '\bSmartPointer<',
    '\bLinkedList<',
    '\bLinkedNode<'
)

$targets = @(
    'SimpleC',
    'IL'
)

$args = @()
foreach ($pattern in $patterns)
{
    $args += '-e'
    $args += $pattern
}
foreach ($target in $targets)
{
    $args += $target
}

$result = & rg -n --no-heading --color never @args
if ($LASTEXITCODE -eq 0)
{
    Write-Host 'Found forbidden CoreLib dependency markers in main compiler chain:' -ForegroundColor Red
    Write-Host $result
    exit 1
}

if ($LASTEXITCODE -eq 1)
{
    Write-Host 'OK: no forbidden CoreLib dependency markers found in SimpleC/IL.' -ForegroundColor Green
    exit 0
}

Write-Host 'Failed to run rg for dependency guard scan.' -ForegroundColor Red
exit $LASTEXITCODE
