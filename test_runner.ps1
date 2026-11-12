param(
    [string]$Emulator = ".\out\build\GCC 15.2.0 x86_64-w64-mingw32 (ucrt64)\gb-emu-cpp.exe",
    [string]$TestRoot = ".\rom\Test Roms\mooneye-test-suite"
)

$root = (Resolve-Path $TestRoot).Path
$skipFolders = 'madness', 'manual-only', 'misc', 'utils'

$results = foreach ($rom in Get-ChildItem -Path $root -Recurse -Filter *.gb) {
    $relative = $rom.FullName.Substring($root.Length + 1)

    # Skip folders that aren't automatic DMG tests
    if ($skipFolders | Where-Object { $relative -like "$_\*" }) { continue }

    # A suffix after the last '-' names the hardware; keep only DMG ones
    if ($rom.BaseName -match '-([A-Za-z0-9]+)$' -and
        $Matches[1] -cnotmatch '^G|dmgABC') { continue }
    Write-Host "Running test: $relative"
    & $Emulator --test-mode --rom $rom.FullName *> $null

    $status = switch ($LASTEXITCODE) {
        0 { 'PASS' }
        1 { 'FAIL' }
        2 { 'NO RESULT' }
        3 { 'NO ROM' }
        default { "EXIT $LASTEXITCODE" }
    }
    [pscustomobject]@{ Test = $relative; Result = $status }
}

$results | Format-Table -AutoSize
$results | Group-Object Result | Select-Object Name, Count | Format-Table -AutoSize