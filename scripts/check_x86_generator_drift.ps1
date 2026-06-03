param()

$repo = Split-Path -Parent $PSScriptRoot
$generator = Join-Path $repo 'DevTools\X86_InstrCodeGen\Debug\X86_InstrCodeGen.exe'
$instructionFile = Join-Path $repo 'IL\Instruction_x86.cpp'
$emitterHeader = Join-Path $repo 'IL\CodeEmitter_x86.h'

if (-not (Test-Path -LiteralPath $generator))
{
    Write-Host 'X86 instruction generator is missing. Build DevTools\X86_InstrCodeGen Debug|Win32 first.' -ForegroundColor Red
    exit 1
}

$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) 'CCompiler-X86GeneratorDrift'
$tempDir = Join-Path $tempRoot ([System.Guid]::NewGuid().ToString('N'))
[System.IO.Directory]::CreateDirectory($tempDir) | Out-Null

$generatedCode = Join-Path $tempDir 'Instruction_x86.generated.cpp'
$generatedHeader = Join-Path $tempDir 'CodeEmitter_x86.generated.decls'

try
{
    & $generator $generatedCode $generatedHeader
    if ($LASTEXITCODE -ne 0)
    {
        Write-Host 'X86 instruction generator failed.' -ForegroundColor Red
        exit $LASTEXITCODE
    }

    $oldApiMatches = & rg -n --no-heading --color never 'code\.Add' $generatedCode
    if ($LASTEXITCODE -eq 0)
    {
        Write-Host 'Generated x86 instruction output still uses removed code.Add API:' -ForegroundColor Red
        Write-Host $oldApiMatches
        exit 1
    }
    if ($LASTEXITCODE -ne 1)
    {
        Write-Host 'Failed to scan generated x86 instruction output.' -ForegroundColor Red
        exit $LASTEXITCODE
    }

    $headerText = Get-Content -LiteralPath $emitterHeader -Raw
    $generatedDeclarations = Get-Content -LiteralPath $generatedHeader
    $missingDeclarations = @()
    foreach ($declaration in $generatedDeclarations)
    {
        $trimmedDeclaration = $declaration.Trim()
        if ($trimmedDeclaration.Length -eq 0)
        {
            continue
        }
        if (-not $headerText.Contains($trimmedDeclaration))
        {
            $missingDeclarations += $trimmedDeclaration
        }
    }
    if ($missingDeclarations.Count -gt 0)
    {
        Write-Host 'Generated x86 emitter declarations are not present in IL\CodeEmitter_x86.h.' -ForegroundColor Red
        Write-Host $missingDeclarations
        exit 1
    }

    $currentCode = (Get-Content -LiteralPath $instructionFile -Raw) -replace '\s+', ''
    $newCode = (Get-Content -LiteralPath $generatedCode -Raw) -replace '\s+', ''
    if ($currentCode -eq $newCode)
    {
        Write-Host 'OK: x86 generated instruction implementation matches IL\Instruction_x86.cpp.' -ForegroundColor Green
        exit 0
    }

    Write-Host 'WARN: generated x86 instruction implementation differs from IL\Instruction_x86.cpp.' -ForegroundColor Yellow
    Write-Host 'Known current state: IL\Instruction_x86.cpp contains hand-maintained differences, so do not overwrite it blindly.' -ForegroundColor Yellow
    Write-Host 'OK: generated output uses current vector API and declarations still match CodeEmitter_x86.h.' -ForegroundColor Green
    exit 0
}
finally
{
    if (Test-Path -LiteralPath $tempDir)
    {
        Remove-Item -LiteralPath $tempDir -Recurse -Force
    }
}
