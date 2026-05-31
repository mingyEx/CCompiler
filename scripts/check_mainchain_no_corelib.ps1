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
    'IL',
    'DevTools\X86_InstrCodeGen'
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
    Write-Host 'Found forbidden CoreLib dependency markers in protected compiler chain:' -ForegroundColor Red
    Write-Host $result
    exit 1
}

if ($LASTEXITCODE -eq 1)
{
    Write-Host 'OK: no forbidden CoreLib dependency markers found in SimpleC/IL/DevTools.' -ForegroundColor Green
    exit 0
}

Write-Host 'Failed to run rg for dependency guard scan.' -ForegroundColor Red
exit $LASTEXITCODE
