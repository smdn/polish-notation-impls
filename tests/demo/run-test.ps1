#!/usr/bin/env pwsh
# SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT

$test_output_dir = $([System.IO.Path]::Join($PSScriptRoot, "test-output"))
$testcases = @()

$input_expression_calculable = "2 + 5 * 3 - 4"

$testcases += (
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-preorder"
    "InputExpression" = $input_expression_calculable
    "OutputSvgFileName" = "calculable.traverse-preorder.generated.svg"
    "ExpectedSvgFileName" = "calculable.traverse-preorder.expected.svg"
    "OutputDiffFileName" = "calculable.traverse-preorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-inorder"
    "InputExpression" = $input_expression_calculable
    "OutputSvgFileName" = "calculable.traverse-inorder.generated.svg"
    "ExpectedSvgFileName" = "calculable.traverse-inorder.expected.svg"
    "OutputDiffFileName" = "calculable.traverse-inorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-postorder"
    "InputExpression" = $input_expression_calculable
    "OutputSvgFileName" = "calculable.traverse-postorder.generated.svg"
    "ExpectedSvgFileName" = "calculable.traverse-postorder.expected.svg"
    "OutputDiffFileName" = "calculable.traverse-postorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "calculate"
    "InputExpression" = $input_expression_calculable
    "OutputSvgFileName" = "calculable.calculation.generated.svg"
    "ExpectedSvgFileName" = "calculable.calculation.expected.svg"
    "OutputDiffFileName" = "calculable.calculation.svg.diff"
  }
)

$input_expression_incalculable = "1 + 2 + X = Y + 3 + 4"

$testcases += (
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-preorder"
    "InputExpression" = $input_expression_incalculable
    "OutputSvgFileName" = "incalculable.traverse-preorder.generated.svg"
    "ExpectedSvgFileName" = "incalculable.traverse-preorder.expected.svg"
    "OutputDiffFileName" = "incalculable.traverse-preorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-inorder"
    "InputExpression" = $input_expression_incalculable
    "OutputSvgFileName" = "incalculable.traverse-inorder.generated.svg"
    "ExpectedSvgFileName" = "incalculable.traverse-inorder.expected.svg"
    "OutputDiffFileName" = "incalculable.traverse-inorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-postorder"
    "InputExpression" = $input_expression_incalculable
    "OutputSvgFileName" = "incalculable.traverse-postorder.generated.svg"
    "ExpectedSvgFileName" = "incalculable.traverse-postorder.expected.svg"
    "OutputDiffFileName" = "incalculable.traverse-postorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "calculate"
    "InputExpression" = $input_expression_incalculable
    "OutputSvgFileName" = "incalculable.calculation.generated.svg"
    "ExpectedSvgFileName" = "incalculable.calculation.expected.svg"
    "OutputDiffFileName" = "incalculable.calculation.svg.diff"
  }
)

New-Item -Path $test_output_dir -ItemType Directory -ErrorAction SilentlyContinue > $null

$exit_code = 0

foreach ($testcase in $testcases) {
  Write-Host "Test case: '$($testcase.InputExpression)' ($($testcase.VisualizationMode))"

  $output_path_svg = [System.IO.Path]::Join($test_output_dir, $testcase.OutputSvgFileName)
  $output_path_diff = [System.IO.Path]::Join($test_output_dir, $testcase.OutputDiffFileName)

  $generator_args = @(
    "--input-expression", $testcase.InputExpression,
    "--visualization-mode", $testcase.VisualizationMode,
    "--output-path", $output_path_svg
  )

  # generate SVG
  $cmd_generate_svg = $([System.IO.Path]::Join($PSScriptRoot, "generate-svg.js"))
  $cmd_format_xml = $([System.IO.Path]::Join($PSScriptRoot, "tools/format-xml.csx"))

  & $cmd_generate_svg @generator_args
  & $cmd_format_xml $output_path_svg

  # get diff between the generated SVG and the expected one
  $diff_args = @(
    "--no-pager", # this must appear before the `diff` subcommand
    "diff",
    "--no-index",
    "--no-color",
    "-U9999",
    "--",
    [System.IO.Path]::Join($PSScriptRoot, "expected-result", $testcase.ExpectedSvgFileName),
    $output_path_svg
  )

  git @diff_args | Tee-Object -Variable difference | Out-File -Encoding UTF-8 -FilePath $output_path_diff

  "generated $output_path_diff"

  # print remarkable differences
  $number_of_differences = 0

  foreach ($line in $($difference -split [System.Environment]::NewLine)) {
    if ($line -match '^(?<change>\+|\-)\s+(?<line>.+)$') {
      $change = $Matches.change
      $line_diff = $matches.line

      # ignore the differences of root <svg> attributes
      if ($line -match '^(\+|\-) [a-z\-]+\=\"') {
        continue
      }

      # ignore the differences in coordinate-related attributes
      if (
        $line_diff -match '^(x|y|rx|ry|width|height)=\"(\-?\d+|\-?\d+\.\d+(e(\+|\-)\d+)?)\"' -or
        $line_diff -match '^(d|path)=\"M ' -or
        $line_diff -match '^viewBox=\"' -or
        $line_diff -match '^transform=\"translate\('
      ) {
        continue
      }

      $number_of_differences++;

      if ($change -eq '+') {
        Write-Host $line -ForegroundColor Green
      }
      else {
        Write-Host $line -ForegroundColor Red
      }
    }
  }

  if ($number_of_differences -eq 0) {
    Write-Host -ForegroundColor Green "???? No differences."
  }
  else {
    Write-Host -ForegroundColor Red "???? There are $number_of_differences lines of difference."
  }

  $exit_code += $number_of_differences
}

exit $exit_code
