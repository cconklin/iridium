require 'whittle'

class ParseError < StandardError
end

class Parser < Whittle::Parser

  rule("+") % :left ^ 3
  rule("-") % :left ^ 3
  rule("*") % :left ^ 4
  rule("/") % :left ^ 4
  rule("=") % :left ^ 1
  rule(".") % :right ^ 5
  rule("<<") ^ 3
  rule(">>") ^ 3
  rule("(") ^ 6
  rule(")") ^ 6
  rule("[") ^ 6
  rule("]") ^ 6
  rule("{") ^ 6
  rule("}") ^ 6
  rule("=>") ^ 1
  rule("==") ^ 2
  rule("<=") ^ 2
  rule(">=") ^ 2
  rule(">") ^ 2
  rule("<") ^ 2
  rule("!=") ^ 2
  rule("===") ^ 3
  rule("!==") ^ 3

  rule(",") ^ 6

  rule("return") ^ 1

  rule("require") ^ 1
  rule("nofunction") ^ 1
  rule("nomethod") ^ 1
  rule("function") ^ 1
  rule("method") ^ 1
  rule("private") ^ 1
  rule("module") ^ 1
  rule("class") ^ 1
  rule("if") ^ 1
  rule("unless") ^ 1
  rule("else") ^ 1
  rule("elseif") ^ 1
  rule("while") ^ 1
  rule("begin") ^ 1
  rule("rescue") ^ 1
  rule("ensure") ^ 1

  rule("->") ^ 6
  rule("end") ^ 6

  rule(:wsp => /\s+/).skip!
  rule(:comment => /#.*$/).skip!

  rule(:identifier => /[a-z_][a-zA-Z0-9_]*[\?!]?/).as { |n| n.to_sym }
  rule(:constant => /[A-Z][a-zA-Z0-9_]*/).as { |n| n.to_sym }
  rule(:sym_key => /[a-zA-Z_][a-zA-Z0-9_]*:/).as { |n| ":#{n[0...-1]}".to_sym }
  
  rule(:int => /[+-]?[0-9]+/).as { |n| n.to_i }
  rule(:flt => /[+-]?[0-9]+\.[0-9]+/).as { |n| n.to_f }
  rule(:str => /"(\\"|[^"])*"/).as { |n| n.to_s[1...-1] }
  rule(:array) do |r|
    r["[", :args, "]"].as { |_1, a, _2| [:array, a] }
  end
  rule(:atom => /:[a-zA-Z0-9_]*/).as { |n| n.to_sym }

  rule(:dict_elements) do |r|
    r[:expr, "=>", :expr].as { |a, _, b| {a => b} }
    r[:expr, "=>", :expr, ",", :dict_elements].as { |a, _1, b, _2, c| {a => b}.merge(c) }
    r[:sym_key, :expr].as { |a, b| { a => b} }
    r[:sym_key, :expr, ",", :dict_elements].as { |a, b, _, c| {a => b}.merge(c) }
    r[].as { {} }
  end

  rule(:dictionary) do |r|
    r["{", :dict_elements, "}"].as { |_1, a, _2| [:dictionary, a] }
  end

  rule(:arg) do |r|
    r[:expr].as { |a| a }
    r[:splat].as { |a| a }
  end

  rule(:splat) do |r|
    r["*", :expr].as { |_, a| [:destructure, a] }
  end

  rule(:args) do |r|
    r[:arg, ",", :args].as { |a, _, b| [a] + b }
    r[:arg].as { |a| [a] }
    r[].as { [] }
  end

  rule(:expr) do |r|
    r[:expr, :lambda].as { |a, b| [:"()", a, [b]] }
    r[:expr, "(", :args, ")", :lambda].as { |a, _1, b, _2, c| [:"()", a, b + [c]] }
    r[:expr, "(", :args, ")"].as { |a, _1, b, _2| [:"()", a, b] }
    r[:expr, ">>", :expr].as { |a, _, b| [:>>, a, b] }
    r[:expr, "<<", :expr].as { |a, _, b| [:<<, a, b] }
    r[:expr, "+", :expr].as { |a, _, b| [:+, a, b] }
    r[:expr, "-", :expr].as { |a, _, b| [:-, a, b] }
    r[:expr, "/", :expr].as { |a, _, b| [:/, a, b] }
    r[:expr, "*", :expr].as { |a, _, b| [:*, a, b] }
    r[:expr, "===", :expr].as { |a, _, b| [:===, a, b] }
    r[:expr, "==", :expr].as { |a, _, b| [:==, a, b] }
    r[:expr, ">", :expr].as { |a, _, b| [:>, a, b] }
    r[:expr, "<", :expr].as { |a, _, b| [:<, a, b] }
    r[:expr, "<=", :expr].as { |a, _, b| [:<=, a, b] }
    r[:expr, ">=", :expr].as { |a, _, b| [:>=, a, b] }
    r[:expr, "!=", :expr].as { |a, _, b| [:!=, a, b] }
    r[:expr, "!==", :expr].as { |a, _, b| [:"!==", a, b] }
    r["(", :expr, ")"].as { |_1, e, _2| e }
    r[:array].as { |n| n }
    r[:dictionary].as { |n| n }
    r[:flt].as { |n| n }
    r[:int].as { |n| n }
    r[:str].as { |n| n }
    r[:atom].as { |n| n }
    r[:identifier].as { |n| n }
    r[:constant].as { |n| n }
    r[:index_expr].as { |n| n }
    r[:attr_expr].as { |n| n }
    r[:lambda].as { |n| n }
  end

  rule(:index_expr) do |r|
    r[:expr, "[", :args, "]"].as { |a, _1, b, _2| [:"[]", a, b] }
  end

  rule(:attr_expr) do |r|
    r[:expr, ".", :identifier].as { |a, _, b| [:".", a, b] }
    r[:expr, ".", :constant].as { |a, _, b| [:".", a, b] }
    r[:expr, ".", "class"].as { |a, _, b| [:".", a, :class] }
    r[:expr, ".", "module"].as { |a, _, b| [:".", a, :module] }
  end

  rule(:lambda_open) do |r|
    r["->"]. as { [] }
    r["->", :parameter_list]. as { |_, a| a }
  end

  rule(:lambda) do |r|
    r[:lambda_open, :block, "end"].as { |a, b, _2| [:lambda, a, b] }
  end

  rule(:assignment) do |a|
    a[:identifier, "=", :expr].as { |a, _, b| [:"=", a, b] }
    # Extract expr and args from the index expr: [:[], expr, args]
    a[:index_expr, "=", :expr].as { |a, _, b| [:insert, a[1], a[2], b] }
    a[:attr_expr, "=", :expr].as { |a, _, b| [:set, a[1], a[2], b] }
  end

  rule(:return) do |r|
    r["return", :expr].as { |_, e| [:return, e] }
  end

  rule(:statement) do |s|
    s[:module].as { |m| m }
    s[:class].as { |m| m }
    s[:assignment].as { |as| as }
    s[:expr].as { |e| e }
    s[:if_block].as { |e| e }
    s[:while_block].as { |e| e }
    s[:unless_block].as { |e| e }
    s[:begin_block].as { |b| b }
    s[:function].as { |f| f }
    s[:method].as { |f| f }
    s[:return].as { |r| r }
    s["require", :str].as { |_, e| [:require, e] }
    s["nofunction", :identifier].as { |_, e| [:nofunction, e] }
    s["nomethod", :identifier].as { |_, e| [:nomethod, e] }
  end

  rule(:block) do |b|
    b[:statement, :block].as { |s, bl| [s] + bl }
    b[].as { [] }
  end

  rule(:func_open) do |r|
    r["function", :identifier, :parameter_list].as { |_, a, b| [:function, a, b] }
    r["function", :identifier].as { |_, a| [:function, a, []] }
  end

  rule(:meth_open) do |r|
    r["method", :identifier, :parameter_list].as { |_, a, b| [:method, a, b] }
    r["method", :identifier].as { |_, a| [:method, a, []] }
  end

  rule(:private_meth_open) do |r|
    r["private", "method", :identifier, :parameter_list].as { |_1, _2, a, b| [:private_method, a, b] }
    r["private", "method", :identifier].as { |_1, _2, a| [:private_method, a, []] }
  end

  rule(:private_func_open) do |r|
    r["private", "function", :identifier, :parameter_list].as { |_1, _2, a, b| [:private_function, a, b] }
    r["private", "function", :identifier].as { |_1, _2, a| [:private_function, a, []] }
  end

  rule(:function) do |r|
    r[:func_open, :block, "end"].as { |a, b, _2| [*a, b] }
    r[:private_func_open, :block, "end"].as { |a, b, _2| [*a, b] }
  end

  rule(:method) do |r|
    r[:meth_open, :block, "end"].as { |a, b, _2| [*a, b] }
    r[:private_meth_open, :block, "end"].as { |a, b, _2| [*a, b] }
  end

  rule(:while_block) do |r|
    r[:while_cond, :block, "end"].as { |a, b, _| [:while, a, b] }
  end

  rule(:while_cond) do |r|
    r["while", :expr].as { |_, a| a }
  end

  rule(:if_cond) do |r|
    r["if", :expr].as { |_, a| a }
  end

  rule(:parameter) do |r|
    r[:identifier].as { |a| a }
    r[:identifier, "=", :expr].as { |a, _, b| { a => b } }
    r["*", :identifier].as { |_, a| [:destructure, a] }
  end

  rule(:parameters) do |r|
    r[:parameter, ",", :parameters].as { |a, _, b| [a] + b }
    r[:parameter].as { |a| [a] }
    r[].as { [] }
  end

  rule(:parameter_list) do |r|
    r["(", :parameters, ")"].as { |_1, a, _2| a }
  end

  rule(:else_stmt) do |r|
    r["else", :block].as { |_, a| [:else, a] }
    r[]
  end

  rule(:if_stmt) do |r|
    r[:if_cond, :block].as { |a, b| [a, b] }
  end

  rule(:elseif_cond) do |r|
    r["elseif", :expr].as { |_, a| a }
  end

  rule(:elseif_stmt) do |r|
    r[:elseif_cond, :block].as { |a, b| [a, b] }
  end

  rule(:elseif_stmts) do |r|
    r[:elseif_stmt, :elseif_stmts].as { |a, b| [a] + b }
    r[].as { [] }
  end

  rule(:unless_cond) do |r|
    r["unless", :expr].as { |_, a| a }
  end

  rule(:unless_stmt) do |r|
    r[:unless_cond, :block].as { |a, b| [a, b] }
  end

  rule(:unless_block) do |r|
    r[:unless_stmt, :else_stmt, "end"].as { |a, b, _2| [:unless, [a, b].compact] }
  end

  rule(:if_block) do |r|
    r[:if_stmt, :elseif_stmts, :else_stmt, "end"].as { |a, b, c, _2| [:if, [a, *b, c].compact] }
  end

  rule(:class_header) do |r|
    r["class", :constant].as { |_, a| [:class, a, nil] }
    r["class", :constant, "<", :constant].as { |_1, a, _2, b| [:class, a, b] }
  end

  rule(:class) do |r|
    r[:class_header, :block, "end"].as { |a, b, _| [*a, b] }
  end

  rule(:module) do |m|
    m["module", :constant, :block, "end"].as { |_1, a, b, _2| [:module, a, b] }
  end

  rule(:begin_block) do |r|
    r[:begin_stmt, :rescue_stmts, "end"].as { |a, b, _| [*a, b, []] }
    r[:begin_stmt, :rescue_stmts, :ensure_stmt, "end"].as { |a, b, c, _| [*a, b, c] }
  end

  rule(:begin_stmt) do |r|
    r["begin", :block].as { |_, a| [:begin, a] }
  end

  rule(:rescue_stmts) do |r|
    r[:rescue_stmt, :rescue_stmts].as { |a, b| a.merge(b) }
    r[].as { {} }
  end

  rule(:rescue_stmt) do |r|
    r["rescue", :constant, :block].as { |_, a, b| { a => [nil, b] } }
    r["rescue", :constant, "=>", :identifier, :block].as { |_1, a, _2, b, c| { a => [b, c] } }
    r["rescue", :identifier, :block].as { |_, a, b| { a => [nil, b] } }
    r["rescue", :identifier, "=>", :identifier, :block].as { |_1, a, _2, b, c| { a => [b, c] } }
  end

  rule(:ensure_stmt) do |r|
    r["ensure", :block].as { |_, a| a }
  end

  start(:block)

  def parse(*args)
    super.tap { |tree| verify(tree) }
  rescue => e
    raise ParseError, e
  end

  # when :require
  def verify(tree)
    tree.each do |stmt|
      if stmt.is_a?(Array)
        case stmt[0]
        when :module then verify_module(stmt[2])
        when :class then verify_class(stmt[3])
        when :function then verify_function(stmt[3])
        when :lambda then verify_function(stmt[2])
        when :if, :unless then stmt[1].each { |s| verify(s[1]) }
        when :while, :until then verify(stmt[2])
        when :begin
          verify(stmt[1])
          stmt[2].each_value { |v| verify(v[1]) }
          verify(stmt[3])
        when :nomethod, :method, :private_function, :private_method, :return then raise ParseError, "#{stmt[0]} not allowed at top scope"
        end
      end
    end
  end

  def verify_module(tree)
    tree.each do |stmt|
      if stmt.is_a?(Array)
        case stmt[0]
        when :module then verify_module(stmt[2])
        when :class then verify_class(stmt[3])
        when :function, :private_function, :method, :private_method then verify_function(stmt[3])
        when :lambda then verify_function(stmt[2])
        when :if, :unless then stmt[1].each { |s| verify_module(s[1]) }
        when :while, :until then verify_module(stmt[2])
        when :begin
          verify_module(stmt[1])
          stmt[2].each_value { |v| verify_module(v[1]) }
          verify_module(stmt[3])
        when :require, :return then raise ParseError, "#{stmt[0]} not allowed in module"
        end
      end
    end
  end

  def verify_class(tree)
    tree.each do |stmt|
      if stmt.is_a?(Array)
        case stmt[0]
        when :function, :private_function, :method, :private_method then verify_function(stmt[3])
        when :lambda then verify_function(stmt[2])
        when :if, :unless then stmt[1].each { |s| verify_class(s[1]) }
        when :while, :until then verify_class(stmt[2])
        when :begin
          verify_class(stmt[1])
          stmt[2].each_value { |v| verify_class(v[1]) }
          verify_class(stmt[3])
        when :module, :class, :require, :return then raise ParseError, "#{stmt[0]} not allowed in class"
        end
      end
    end
  end

  def verify_function(tree)
    tree.each do |stmt|
      if stmt.is_a?(Array)
        case stmt[0]
        when :lambda then verify_function(stmt[2])
        when :if, :unless then stmt[1].each { |s| verify_function(s[1]) }
        when :while, :until then verify_function(stmt[2])
        when :begin
          verify_function(stmt[1])
          stmt[2].each_value { |v| verify_function(v[1]) }
          verify_function(stmt[3])
        when :require, :function, :private_function, :private_method, :method, :class, :module then raise ParseError, "#{stmt[0]} not allowed in function body"
        end
      end
    end
  end

end

