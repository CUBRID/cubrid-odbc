#
# Turn the TRX files written by test.bat into a failures-only report.
#
# Prints one block per test run with its counters and, for every test that did
# not pass, the assertion message and the line it came from.  Passing tests are
# not printed at all - a green run says so in a single line.
#
# When it runs inside GitHub Actions ($GITHUB_STEP_SUMMARY is set) the same
# report is appended to the job summary, with the captured test output of each
# failed test folded into a <details> block.
#
# Exits 1 when a test failed, when a run named by -Expect left no TRX file
# behind, or when such a run executed no test at all.  A summed total is not
# enough on its own: one suite that never ran would otherwise hide behind the
# tests of the other one.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File script\report_test_result.ps1
#   powershell -ExecutionPolicy Bypass -File script\report_test_result.ps1 -Path C:\logs
#   powershell -ExecutionPolicy Bypass -File script\report_test_result.ps1 -Expect unit_test
#
[CmdletBinding()]
param(
    [string] $Path = '',
    # the TRX files test.bat has to leave behind, by base name; each of them
    # must exist and must have executed at least one test
    [string[]] $Expect = @('unit_test', 'unit_test_cpp'),
    # how much of the captured output of a failed test to keep in the summary
    [int] $OutputLimit = 4000
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrEmpty($Path)) {
    $Path = Split-Path -Parent $PSScriptRoot
}

# ------------------------------------------------------------------- parsing

function Get-TestRun
{
    param([System.IO.FileInfo] $File)

    $xml = New-Object System.Xml.XmlDocument
    $xml.Load($File.FullName)

    $counters = $xml.TestRun.ResultSummary.Counters
    $results = @($xml.TestRun.Results.UnitTestResult)

    $failed = @()
    foreach ($r in $results) {
        if ($null -eq $r -or $r.outcome -eq 'Passed') {
            continue
        }

        $failed += [pscustomobject]@{
            Name       = $r.testName
            Outcome    = $r.outcome
            Duration   = $r.duration
            Message    = [string] $r.Output.ErrorInfo.Message
            StackTrace = [string] $r.Output.ErrorInfo.StackTrace
            StdOut     = [string] $r.Output.StdOut
        }
    }

    # a run whose tests were all skipped, or that listed none at all, has not
    # tested anything; 'executed' says so directly where the TRX carries it
    $passed = [int] $counters.passed
    $failedCount = [int] $counters.failed + [int] $counters.error + [int] $counters.aborted
    $executed = if ($null -ne $counters.executed) { [int] $counters.executed } else { $passed + $failedCount }

    return [pscustomobject]@{
        Name     = $File.BaseName
        Total    = [int] $counters.total
        Executed = $executed
        Passed   = $passed
        Failed   = $failedCount
        Skipped  = [int] $counters.notExecuted
        Results  = $failed
    }
}

function Format-Indented
{
    param([string] $Text, [string] $Indent = '    ')

    if ([string]::IsNullOrWhiteSpace($Text)) {
        return @()
    }

    return ($Text -split "`r?`n" | Where-Object { $_.Trim() -ne '' } | ForEach-Object { "$Indent$($_.Trim())" })
}

# ---------------------------------------------------------------------- main

$files = @(Get-ChildItem -LiteralPath $Path -Filter '*.trx' -File -ErrorAction SilentlyContinue |
    Sort-Object Name)

Write-Host ''
Write-Host '=========================== TEST SUMMARY ==========================='

if ($files.Count -eq 0) {
    Write-Host "No TRX file is found in $Path, the tests did not run."
}

$runs = @()
foreach ($f in $files) {
    $runs += Get-TestRun $f
}

# [int] because Measure-Object over no run at all gives $null, and $null is
# not equal to 0
$totalFailed = [int] ($runs | Measure-Object -Property Failed -Sum).Sum
$totalRun = [int] ($runs | Measure-Object -Property Total -Sum).Sum

# Every expected run is checked on its own. Summing the counters would let a
# suite that never started pass on the tests of the other one.
$missing = @()
$emptyRun = @()
foreach ($want in $Expect) {
    $run = $runs | Where-Object { $_.Name -eq $want } | Select-Object -First 1
    if ($null -eq $run) {
        $missing += $want
    }
    elseif ($run.Executed -eq 0) {
        $emptyRun += $want
    }
}

foreach ($run in $runs) {
    Write-Host ''
    Write-Host ("{0}: total {1}, passed {2}, failed {3}, skipped {4}" -f
        $run.Name, $run.Total, $run.Passed, $run.Failed, $run.Skipped)

    foreach ($r in $run.Results) {
        Write-Host ''
        Write-Host ("  {0}  {1} ({2})" -f $r.Outcome.ToUpper(), $r.Name, $r.Duration)
        Format-Indented $r.Message | ForEach-Object { Write-Host $_ }
        Format-Indented $r.StackTrace | ForEach-Object { Write-Host $_ }
    }
}

Write-Host ''
foreach ($m in $missing) {
    Write-Host ("  MISSING  {0}.trx was never written - that suite did not run." -f $m)
}
foreach ($m in $emptyRun) {
    Write-Host ("  EMPTY    {0}.trx executed no test." -f $m)
}
if ($missing.Count -gt 0 -or $emptyRun.Count -gt 0) {
    Write-Host ''
}

if ($totalRun -eq 0) {
    Write-Host 'No test ran.'
}
elseif ($totalFailed -gt 0) {
    Write-Host ("{0} of {1} test(s) failed." -f $totalFailed, $totalRun)
}
elseif ($missing.Count -gt 0 -or $emptyRun.Count -gt 0) {
    Write-Host ("{0} test(s) passed, but a suite above did not run." -f $totalRun)
}
else {
    Write-Host ("All {0} test(s) passed." -f $totalRun)
}
Write-Host '===================================================================='

# ------------------------------------------------------- GitHub job summary

if (-not [string]::IsNullOrEmpty($env:GITHUB_STEP_SUMMARY)) {
    $md = New-Object System.Collections.Generic.List[string]

    $md.Add('## CUBRID Windows ODBC test')
    $md.Add('')
    $md.Add('| run | total | executed | passed | failed | skipped |')
    $md.Add('|---|---:|---:|---:|---:|---:|')
    foreach ($run in $runs) {
        $md.Add(('| {0} | {1} | {2} | {3} | {4} | {5} |' -f
            $run.Name, $run.Total, $run.Executed, $run.Passed, $run.Failed, $run.Skipped))
    }
    foreach ($m in $missing) {
        $md.Add(('| {0} | *no TRX file* | | | | |' -f $m))
    }
    $md.Add('')

    if ($missing.Count -gt 0 -or $emptyRun.Count -gt 0) {
        $md.Add('### a suite did not run')
        $md.Add('')
        foreach ($m in $missing) {
            $md.Add(('- `{0}.trx` was never written - that suite did not run.' -f $m))
        }
        foreach ($m in $emptyRun) {
            $md.Add(('- `{0}.trx` executed no test.' -f $m))
        }
        $md.Add('')
    }

    if ($totalFailed -eq 0 -and $missing.Count -eq 0 -and $emptyRun.Count -eq 0) {
        $md.Add(('All {0} test(s) passed.' -f $totalRun))
    }
    elseif ($totalFailed -eq 0) {
        $md.Add(('{0} test(s) passed, but a suite above did not run.' -f $totalRun))
    }
    else {
        $md.Add(('### {0} of {1} test(s) failed' -f $totalFailed, $totalRun))
        $md.Add('')

        foreach ($run in $runs) {
            foreach ($r in $run.Results) {
                $md.Add(('#### {0} / {1} ({2})' -f $run.Name, $r.Name, $r.Outcome))
                $md.Add('')
                $md.Add('```')
                $md.Add($r.Message.Trim())
                if (-not [string]::IsNullOrWhiteSpace($r.StackTrace)) {
                    $md.Add($r.StackTrace.Trim())
                }
                $md.Add('```')
                $md.Add('')

                if (-not [string]::IsNullOrWhiteSpace($r.StdOut)) {
                    $out = $r.StdOut.Trim()
                    if ($out.Length -gt $OutputLimit) {
                        $out = $out.Substring(0, $OutputLimit) + "`n... (truncated)"
                    }
                    $md.Add('<details><summary>test output</summary>')
                    $md.Add('')
                    $md.Add('```')
                    $md.Add($out)
                    $md.Add('```')
                    $md.Add('')
                    $md.Add('</details>')
                    $md.Add('')
                }
            }
        }
    }

    # Add-Content -Encoding utf8 of Windows PowerShell would put a BOM in
    # front of the first line of the summary
    $utf8 = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::AppendAllText($env:GITHUB_STEP_SUMMARY, (($md -join "`n") + "`n"), $utf8)
}

if ($totalFailed -gt 0 -or $totalRun -eq 0 -or
    $missing.Count -gt 0 -or $emptyRun.Count -gt 0) {
    exit 1
}
exit 0
