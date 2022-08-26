#!/usr/bin/env pwsh
# SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT

Param(
  [Parameter()]
  [Alias("v")]
  [switch]
  $RunVerbose = $false,

  [Parameter()]
  [Alias("target-impl")]
  [String]
  $TargetImplementationId = $null
)

function Invoke-Tests {
  param(
    $impl,
    $testsuites,
    $verbose
  )

  $dir_root = [System.IO.Path]::GetFullPath(
    [System.IO.Path]::Join($PSScriptRoot, "../../")
  )

  Write-Host "Run tests with an implementation of $($impl.DisplayName) ..."

  Set-Location $([System.IO.Path]::Join($dir_root, $impl.Directory))

  if ($verbose) {
    Write-Host "working directory: $(Get-Location)"
  }

  # clean-up
  foreach ($cmd in $impl.Commands.Clean) {
    try {
      if ($verbose) {
        Write-Host "$($cmd.Command) $($cmd.Arguments -join ' ')"
      }

      if ($cmd.UseInvokeExpression) {
        Invoke-Expression "$($cmd.Command) $($cmd.Arguments -join ' ')"
      }
      else {
        [void]$(& $cmd.Command $cmd.Arguments)
      }
    }
    catch {
      Write-Error "clean failed ($($cmd.Command) $($cmd.Arguments -join ' '))"
      break
    }
  }

  # build an implementation
  $done_build = $true

  foreach ($cmd in $impl.Commands.Build) {
    try {
      if ($verbose) {
        Write-Host "$($cmd.Command) $($cmd.Arguments -join ' ')"
      }

      if ($cmd.UseInvokeExpression) {
        Invoke-Expression "$($cmd.Command) $($cmd.Arguments -join ' ')"
      }
      else {
        [void]$(& $cmd.Command $cmd.Arguments)
      }
    }
    catch {
      Write-Error "build failed ($($cmd.Command) $($cmd.Arguments -join ' '))"
      $done_build = $false
      break
    }
  }

  if ( ! $done_build ) {
    return $false
  }

  # run test cases
  $psi = New-Object System.Diagnostics.ProcessStartInfo
  $psi.FileName = $impl.Commands.Run.Command
  $psi.Arguments = $impl.Commands.Run.Arguments
  $psi.WorkingDirectory = $(Get-Location)
  $psi.RedirectStandardInput = $true
  $psi.RedirectStandardOutput = $true
  $psi.RedirectStandardError = $true
  $psi.UseShellExecute = $false

  if ($impl.Commands.Run.ResolveCommandPath) {
    $psi.FileName = [System.IO.Path]::GetFullPath($psi.FileName, $(Get-Location))
  }

  if ($verbose) {
    Write-Host "$($impl.Commands.Run.Command) $($impl.Commands.Run.Arguments -join ' ')"
  }

  $ret = Invoke-TestCases $impl $psi $testsuites $verbose

  if ($ret) {
    Write-Host -ForegroundColor Green "🆗 All tests passed"
  }

  return $ret
}

function Invoke-TestCase {
  param (
    $psi,
    $testcase,
    $verbose
  )

  #
  # run test case
  #
  $p = New-Object System.Diagnostics.Process
  $p.StartInfo = $psi

  try {
    [void]$p.Start()
  }
  catch {
    throw [System.InvalidOperationException]::new(
      @(
        "Failed to start test process.",
        "  FileName: '$($psi.FileName)'",
        "  Arguments: '$($psi.Arguments)'",
        "  WorkingDirectory: '$($psi.WorkingDirectory))'"
      ) -join [System.Environment]::NewLine,
      $PSItem.Exception
    )
  }

  if ($null -ne $testcase.Input) {
    [void]$p.StandardInput.WriteLine($testcase.Input)
    [void]$p.StandardInput.Flush()
  }

  [void]$p.StandardInput.Close()

  $p.WaitForExit()
  $result_stdout = $p.StandardOutput.ReadToEnd().TrimEnd()
  $result_stderr = $p.StandardError.ReadToEnd().TrimEnd()
  $result_exitcode = $p.ExitCode

  try {
    #
    # validation result
    #
    $was_unbalanced_bracket = $($result_stderr -match "(?m)^unbalanced bracket:")

    if ($was_unbalanced_bracket -ne [bool]$testcase.ExpectAsUnbalancedBracket) {
      throw "'$($testcase.Input)' is expected as $($testcase.ExpectAsUnbalancedBracket ? 'unbalanced' : 'balanced') bracket expression, but not"
    }

    $was_empty_bracket = $($result_stderr -match "(?m)^empty bracket:")

    if ($was_empty_bracket -ne [bool]$testcase.ExpectAsEmptyBracket) {
      throw "'$($testcase.Input)' is expected as $($testcase.ExpectAsEmptyBracket ? 'empty' : 'non-empty') bracket expression, but not"
    }

    $was_invalid_expression = $($result_stderr -match "(?m)^invalid expression:")

    if ($was_invalid_expression -ne [bool]$testcase.ExpectAsInvalidExpression) {
      throw "'$($testcase.Input)' is expected as $($testcase.ExpectAsInvalidExpression ? 'invalid' : 'valid') expression, but not"
    }

    #
    # calculation result
    #
    if ($testcase.ExpectedCalculationResult) {
      if ($result_stdout -match "(?m)^calculated result: (?<result>.+)$") {
        $actual_calculation_result = $Matches["result"].TrimEnd("`n", "`r") # trim CRLF
      }

      if ($testcase.ExpectedCalculationResult -ne $actual_calculation_result) {
        throw "'$($testcase.Input)' must be calculated to the value '$($testcase.ExpectedCalculationResult)', but was '$($actual_calculation_result ?? '(not calculated)')'"
      }
    }

    if ($testcase.ExpectAsCalculatedExpression) {
      if ($result_stdout -match "(?m)^calculated expression: (?<exp>.+)$") {
        $actual_calculated_expression = $Matches["exp"].TrimEnd("`n", "`r") # trim CRLF
      }

      if ($testcase.ExpectAsCalculatedExpression -ne $actual_calculated_expression) {
        throw "'$($testcase.Input)' is expected to be calculated to the expression '$($testcase.ExpectAsCalculatedExpression)', but was '$($actual_calculated_expression ?? '(calculated)')'"
      }
    }

    #
    # notation transform result
    #
    foreach ($test in @(
      [PSCustomObject]@{
        Notation = "reverse polish notation"
        ExpectedExpression = $testcase.ExpectedPostorderNotation
      },
      [PSCustomObject]@{
        Notation = "polish notation"
        ExpectedExpression = $testcase.ExpectedPreorderNotation
      },
      [PSCustomObject]@{
        Notation = "infix notation"
        ExpectedExpression = $testcase.ExpectedInorderNotation
      }
    )) {
      if ($test.ExpectedExpression) {
        if ($result_stdout -match "(?m)^$($test.Notation): (?<exp>.+)$") {
          $actual_expression = $Matches["exp"].TrimEnd("`n", "`r") # trim CRLF
        }

        if ($test.ExpectedExpression -ne $actual_expression) {
          throw "'$($testcase.Input)' is expected to be transformed to the expression '$($test.ExpectedExpression)' in $($test.Notation), but was '$actual_expression'"
        }
      }
    }

    #
    # exit code
    #
    if (($null -ne $testcase.ExpectedExitCode) -and ($testcase.ExpectedExitCode -ne $result_exitcode)) {
      throw "'$($testcase.Input)' is expected to be exited with exit code $($testcase.ExpectedExitCode), but was $result_exitcode"
    }

    $expect_as_invalid =
      [bool]$testcase.ExpectedExitCode -or
      [bool]$testcase.ExpectAsInvalidExpression -or
      [bool]$testcase.ExpectAsUnbalancedBracket -or
      [bool]$testcase.ExpectAsEmptyBracket

    if (!$expect_as_invalid -and $result_exitcode -ne 0) {
      throw "'$($testcase.Input)' returns unexpected exit code ($result_exitcode)"
    }
  }
  catch {
    Write-Host -ForegroundColor Red "🆖 $_"
    Write-Host -ForegroundColor Yellow $result_stdout
    Write-Host -ForegroundColor Red $result_stderr
    return $false
  }

  #
  # test case passed
  #
  if ($verbose) {
    Write-Host -ForegroundColor Green "🆗 '$($testcase.Input)'"
  }

  return $true
}

function Invoke-TestCases {
  param (
    $impl,
    $psi,
    $testsuites,
    $verbose
  )

  $result = $true

  foreach ($testsuite in $testsuites) {
    if (($null -ne $testsuite.TargetImplementations) -and !$testsuite.TargetImplementations.Contains($impl.ID)) {
      if ($verbose) {
        Write-Host -ForegroundColor Yellow "ℹ️ Test suite '$($testsuite.Name)' is not performed with implementation '$($impl.DisplayName)'"
      }
      continue
    }

    $number_of_failure = 0
    $number_of_ignored = 0

    foreach ($testcase in $testsuite.TestCases) {
      if (($null -ne $testcase.TargetImplementations) -and !$testcase.TargetImplementations.Contains($impl.ID)) {
        if ($verbose) {
          Write-Host -ForegroundColor Yellow "ℹ️ Test case '$($testcase.Input)' is not performed with implementation '$($impl.DisplayName)'"
        }
        $number_of_ignored++
        continue
      }

      $ret = Invoke-TestCase $psi $testcase $verbose

      if (!$ret) {
        $number_of_failure++
        $result = $false
      }
    }

    if ((0 -eq $number_of_ignored) -and (0 -eq $number_of_failure)) {
      Write-Host -ForegroundColor Green "🆗 $($testsuite.TestCases.Length) test cases passed: '$($testsuite.Name)'"
    }
    elseif (0 -eq $number_of_failure) {
      Write-Host -ForegroundColor Green "🆗 $($testsuite.TestCases.Length - $number_of_ignored) test cases passed, $number_of_ignored test cases ignored: '$($testsuite.Name)'"
    }
    else {
      Write-Host -ForegroundColor Red "🆖 Failed $number_of_failure of $($testsuite.TestCases.Length) test cases: '$($testsuite.Name)'"
    }
  }

  return $result
}

#
# main
#

if ($PSVersionTable.PSVersion.Major -lt 7) {
  # requires support for loading .jsonc
  Write-Error "Powershell version 7.0 or greater is required to run tests."
  exit 1
}

$implementations = Get-Content -Path $([System.IO.Path]::Combine($PSScriptRoot, "implementations.jsonc")) | ConvertFrom-Json

if ($TargetImplementationId) {
  $implementations = $implementations | Where-Object ID -eq $TargetImplementationId
}

$implementations = $implementations | Where-Object {
  # '-or' operator does not performs short-circuit evaluation
  ($_.Condition -eq $null) -or (($_.Condition -ne $null) -and (Invoke-Expression $_.Condition))
}

$testsuites = Get-ChildItem -Path $([System.IO.Path]::Combine($PSScriptRoot, "testcases/*.jsonc")) -File |
  # where Name -eq 'should-be-fail.jsonc' |
  ForEach-Object {
    Get-Content -Path $_ | ConvertFrom-Json
  }

$success = $true

foreach ($impl in $implementations) {
  $initial_working_dir = $(Get-Location)

  try {
    $ret = Invoke-Tests $impl $testsuites $RunVerbose
  }
  finally {
    # revert change of working directory
    Set-Location $initial_working_dir
  }

  $success = $success -and $ret
}

exit $success ? 0 : 1
