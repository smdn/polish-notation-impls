#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import subprocess
import re

def exec_command(command, _stdin):
  p = subprocess.Popen(command, stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)

  if _stdin:
    ret = p.communicate(_stdin.encode('utf-8'))
  else:
    ret = p.communicate()

  return [ret[0].decode('utf-8').rstrip(), ret[1].decode('utf-8').rstrip(), p.returncode]

def exec_tests_with(processor, show_output):
  valid_expressions = [
    ["1",                 "1"],
    ["(1)",               "1"],
    ["((1))",             "1"],
    ["1+2",               "3"],
    ["(1+2)",             "3"],
    ["((1+2))",           "3"],
    ["1+(2)",             "3"],
    ["(1)+2",             "3"],
    ["(1)+(2)",           "3"],
    ["((1)+(2))",         "3"],
    ["(((1)+(2)))",       "3"],
    ["((((1))+((2))))",   "3"],
    ["2+5*3-4",           "13"],
    ["(2+5)*3-4",         "17"],
    ["2+5*(3-4)",         "-3"],
    ["2+(5*3-4)",         "13"],
    ["(2+5*3)-4",         "13"],
    ["(2+5)*(3-4)",       "-7"],
    ["((2+(5*3))-4)",     "13"],
    ["1+4*2+(7-3)/2",     "11"],
    ["1+4*(2+(7-3))/2",   "13"],

    # numeric formatting
    ["1*1.5",             "1.5"],
    ["3/2",               "1.5"],
    ["1/2",               "0.5"],
    ["1/4",               "0.25"],
    ["1/8",               "0.125"],
    ["1/3",               "0.333333333333333"],
    ["2/3",               "0.666666666666667"],
    ["10000000000000000/1", "1e+16"],
    ["1/10000000000000000", "1e-16"],
    ["(3/2)*(10000000000000000/1)", "1.5e+16"],

    # expression which contains space
    [" 1+2",              "3"],
    ["1+2 ",              "3"],
    [" 1+2 ",             "3"],
    ["1 +2",              "3"],
    ["1+ 2",              "3"],
    ["1 + 2",             "3"],
    [" 1 + 2 ",           "3"],
    [" ( 1 + 2) ",        "3"],
  ]

  unbalanced_bracket_expressions = [
    # unbalanced brackets
    "(1",
    "1)",
    "(1+2",
    "1+2)",
    "((1+2)",
    "(1+2))",
    "1+(2+3))",
    "1+((2+3)",
    "((1)+(2)",
    "(1)+(2))",

    # empty brackets
    "()",
    "(())",
    "((()))",
    "1=()",
    "()=1",

    # parsed as valid expression currently
    # "x()",
    # "(x())",
  ]

  uncalculated_expressions = [
    ["A", "A"],
    ["A + B", "(A+B)"],
    ["A = B", "(A=B)"],
    ["(A + B) * C", "((A+B)*C)"],
    ["(A + B) = C", "((A+B)=C)"],
    ["(A = B) + C", "((A=B)+C)"], # XXX: invalid notation but valid expression
    ["x = a * (b + c)", "(x=(a*(b+c)))"],
    ["x = 1 + 2", "(x=3)"],
    ["x + 1 + 2 = 3 + 4", "((x+3)=7)"],
    ["x + (1 + 2) = 3 + 4", "((x+3)=7)"],
    ["(x + 1) + 2 = 3 + 4", "(((x+1)+2)=7)"],
    ["1 + 2 + x = 3 + 4", "((1+(2+x))=7)"],
    ["1 + (2 + x) = 3 + 4", "((1+(2+x))=7)"],
    ["(1 + 2) + x = 3 + 4", "((3+x)=7)"],
    ["x = 1 + a", "(x=(1+a))"],
    ["x = 1 = 2", "(x=(1=2))"],
    ["(x = 1) = 2", "((x=1)=2)"],
  ]

  invalid_expressions = [
    "+",
    "+1",
    "1+",
    "1++2",
    "1+(2+)",
    "1+(+2)",
    "1=",
    "=1",
  ]


  for pair in valid_expressions:
    expression      = pair[0]
    expected_result = pair[1]

    calculated = False

    [out, err, retcode] = exec_command(processor, expression)

    for line in out.splitlines():
      result = re.search(r"^calculated result: (.+)$", line)

      if result:
        calculated = True
        actual_result = result.group(1)

        if actual_result != expected_result:
          print("['{}' = {}]".format(expression, expected_result))
          print("  calculated result not matched: expected '{}' but was '{}'".format(expected_result, actual_result))

    if not calculated:
      print("['{}' = {}]".format(expression, expected_result))
      print("  expresion not calculated")

    elif show_output:
      print("['{}' = {}]".format(expression, expected_result))
      print(out)


  for expression in unbalanced_bracket_expressions:
    [out, err, retcode] = exec_command(processor, expression)

    parsed_as_unbalanced_bracket = False

    for line in err.splitlines():
      result = re.search(r"^(unbalanced|empty) bracket:", line)

      if result:
        parsed_as_unbalanced_bracket = True

    if not parsed_as_unbalanced_bracket:
      print("['{}']".format(expression))
      print("  expected to parse as unbalanced or empty bracket, but not")
      print(out)
      print(err)

    elif show_output:
      print("['{}']".format(expression))
      print(out)

  for pair in uncalculated_expressions:
    expression      = pair[0]
    expected_result = pair[1]

    [out, err, retcode] = exec_command(processor, expression)

    parsed_as_uncalculated_expression = False

    [out, err, retcode] = exec_command(processor, expression)

    for line in out.splitlines():
      result = re.search(r"^calculated expression: (.+)$", line)

      if result:
        parsed_as_uncalculated_expression = True
        actual_result = result.group(1)

        if actual_result != expected_result:
          print("['{}' = {}]".format(expression, expected_result))
          print("  calculated expression not matched: expected '{}' but was '{}'".format(expected_result, actual_result))

    if not calculated:
      print("['{}' = {}]".format(expression, expected_result))
      print("  expresion not calculated")

    elif show_output:
      print("['{}' = {}]".format(expression, expected_result))
      print(out)

  for expression in invalid_expressions:
    [out, err, retcode] = exec_command(processor, expression)

    parsed_as_invalid_expression = False

    for line in err.splitlines():
      result = re.search(r"^invalid expression:", line)

      if result:
        parsed_as_invalid_expression = True

    if not parsed_as_invalid_expression:
      print("['{}']".format(expression))
      print("  expected to parse as invalid expression, but not")
      print(out)
      print(err)

    elif show_output:
      print("['{}']".format(expression))
      print(out)

  print("done")


def exec_tests(show_output):
  compiler_and_tester_commands = [
    ["C#", ["mcs", "polish.cs"], ["mono", "polish.exe"], ["rm", "-f", "polish.exe"]],
    ["VB", ["vbnc2", "polish.vb"], ["mono", "polish.exe"], ["rm", "-f", "polish.exe"]],
    ["C", ["gcc", "polish.c", "-o", "polish"], ["./polish"], ["rm", "-f", "./polish"]],
    ["Python", None, ["python3", "polish.py"], None],
    ["JavaScript", None, ["nodejs", "polish.js"], None],
  ]

  for compiler_and_tester_command in compiler_and_tester_commands:
    lang = compiler_and_tester_command[0]

    print("### running tests of '{}' ###".format(lang))

    compile_args = compiler_and_tester_command[1]

    if compile_args:
      [out, err, ret] = exec_command(compile_args, None)

      if ret != 0:
        print("compilation failed: {}".format(lang))
        print(out)
        print(err)

        return False

      print("compiled ('{}')".format(" ".join(compile_args)))


    tester_args = compiler_and_tester_command[2]

    print("running tests... ('{}')".format(" ".join(tester_args)))

    exec_tests_with(tester_args, show_output)
    print()
    print()


    cleanup_args = compiler_and_tester_command[3]

    if cleanup_args:
      [out, err, ret] = exec_command(cleanup_args, None)

      if ret != 0:
        print("clean up failed: {}".format(lang))
        print(out)
        print(err)

exec_tests(show_output = False)

print("all done")

