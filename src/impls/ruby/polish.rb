#!/usr/bin/env ruby
# SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT
class Node
  @expression
  @left
  @right

  attr_reader :expression

  def initialize(expression)
    @expression = expression
  end

  def parse
    pos_operator = get_operator_pos(expression)

    if pos_operator < 0
      @left = nil
      @right = nil
      return
    end

    # left-hand side
    @left = Node.new(remove_bracket(@expression[0, pos_operator]))
    @left.parse

    # right-hand side
    @right = Node.new(remove_bracket(@expression[pos_operator + 1, @expression.length]))
    @right.parse

    # operator
    @expression = @expression[pos_operator, 1]
  end

  def remove_bracket(s)
    return s unless s =~ /^\((.*)\)$/

    removed = $1.dup

    nest = 1

    removed.length.times do |i|
      if removed[i, 1] == '('
        nest += 1
      elsif removed[i, 1] == ')'
        nest -= 1
      end

      return s if nest == 0
    end

    raise("unbalanced bracket: #{s}") unless nest == 1

    if removed[0, 1] == '('
      return remove_bracket(removed)
    else
      return removed
    end
  end

  def get_operator_pos(exp)
    return -1 if exp.to_s.empty?

    pos = -1
    nest = 0
    priority = 0
    lowest_priority = 4

    exp.length.times do |i|
      case exp[i, 1]
        when '=' then priority = 1
        when '+' then priority = 2
        when '-' then priority = 2
        when '*' then priority = 3
        when '/' then priority = 3
        when '(' then nest += 1; next
        when ')' then nest -= 1; next
        else next
      end

      if nest == 0 and priority <= lowest_priority
        lowest_priority = priority
        pos = i
      end
    end

    return pos
  end

  def traverse_postorder
    @left .traverse_postorder if @left
    @right.traverse_postorder if @right

    print @expression
  end

  def traverse_inorder
    print '(' if @left and @right

    @left .traverse_inorder if @left

    print @expression

    @right.traverse_inorder if @right

    print ')' if @left and @right
  end

  def traverse_preorder
    print @expression

    @left .traverse_preorder if @left
    @right.traverse_preorder if @right
  end

  def calculate
    if @left and @right
      left_operand  = @left.calculate()
      right_operand = @right.calculate()

      case @expression
        when '+' then return left_operand + right_operand
        when '-' then return left_operand - right_operand
        when '*' then return left_operand * right_operand
        when '/' then return left_operand / right_operand
        else return 0.0
      end
    else
      return @expression.to_f
    end
  end
end

print 'input expression: '

root = Node.new(gets.chomp.gsub(' ', ''))

print "expression: #{root.expression}\n"

root.parse

print "reverse polish notation: "
root.traverse_postorder
print "\n"

print "infix notation: "
root.traverse_inorder
print "\n"

print "polish notation: "
root.traverse_preorder
print "\n"

print "calculated result: "
print root.calculate
print "\n"

