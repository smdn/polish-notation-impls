#!/usr/bin/env pwsh
# SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT

$input_expression = "2 + 5 * 3 - 4"
$test_output_dir = "test-output"

$testcases = (
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-preorder"
    "InputExpression" = $input_expression
    "OutputElementId" = "polish-demo-expressiontree-traverse-preorder"
    "OutputSvgFileName" = "expressiontree-traverse-preorder.generated.svg"
    "ExpectedSvgFileName" = "expressiontree-traverse-preorder.expected.svg"
    "OutputDiffFileName" = "expressiontree-traverse-preorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-inorder"
    "InputExpression" = $input_expression
    "OutputElementId" = "polish-demo-expressiontree-traverse-inorder"
    "OutputSvgFileName" = "expressiontree-traverse-inorder.generated.svg"
    "ExpectedSvgFileName" = "expressiontree-traverse-inorder.expected.svg"
    "OutputDiffFileName" = "expressiontree-traverse-inorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "traverse-postorder"
    "InputExpression" = $input_expression
    "OutputElementId" = "polish-demo-expressiontree-traverse-postorder"
    "OutputSvgFileName" = "expressiontree-traverse-postorder.generated.svg"
    "ExpectedSvgFileName" = "expressiontree-traverse-postorder.expected.svg"
    "OutputDiffFileName" = "expressiontree-traverse-postorder.svg.diff"
  },
  [PSCustomObject]@{
    "VisualizationMode" = "calculate"
    "InputExpression" = $input_expression
    "OutputElementId" = "polish-demo-expressiontree-calculation"
    "OutputSvgFileName" = "expressiontree-calculation.generated.svg"
    "ExpectedSvgFileName" = "expressiontree-calculation.expected.svg"
    "OutputDiffFileName" = "expressiontree-calculation.svg.diff"
  }
)

New-Item -Path $test_output_dir -ItemType Directory -ErrorAction SilentlyContinue > $null

foreach ($testcase in $testcases) {
  $output_path_svg = [System.IO.Path]::Combine($test_output_dir, $testcase.OutputSvgFileName)
  $output_path_diff = [System.IO.Path]::Combine($test_output_dir, $testcase.OutputDiffFileName)

  $generator_args = @(
    "--input-expression", $testcase.InputExpression,
    "--visualization-mode", $testcase.VisualizationMode,
    "--output-element-id", $testcase.OutputElementId,
    "--output-path", $output_path_svg
  )

  # generate SVG
  ./generate-svg.js @generator_args
  ./tools/format-xml.csx $output_path_svg

  # get diff between the generated SVG and the expected one
  $diff_args = @(
    "--no-pager", # this must appear before the `diff` subcommand
    "diff",
    "--no-index",
    "--no-color",
    "-U9999",
    "--",
    [System.IO.Path]::Combine("expected-result", $testcase.ExpectedSvgFileName),
    $output_path_svg
  )

  git @diff_args | Tee-Object -Variable difference | Out-File -Encoding UTF-8 -FilePath $output_path_diff

  "generated $output_path_diff"

  # print remarkable differences
  foreach ($line in $($difference -split [System.Environment]::NewLine)) {
    if ($line -match '^(?<change>\+|\-)\s+(?<line>.+)$') {
      $change = $Matches.change
      $line_diff = $matches.line

      # ignore the differences in coordinate-related attributes
      if (
        $line_diff -match '^(x|y|rx|ry|width|height)=\"(\-?\d+|\-?\d+\.\d+(e(\+|\-)\d+)?)\"' -or
        $line_diff -match '^(d|path)=\"M ' -or
        $line_diff -match '^viewBox=\"' -or
        $line_diff -match '^transform=\"translate\('
      ) {
        continue
      }

      if ($change -eq '+') {
        Write-Host $line -ForegroundColor Green
      }
      else {
        Write-Host $line -ForegroundColor Red
      }
    }
  }
}
