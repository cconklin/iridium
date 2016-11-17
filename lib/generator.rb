class Generator
  def initialize(callables, tree)
    @callables = callables
    @tree = tree
  end

  def generate_callables
    level = 0;
    @callables.flat_map do |name, tree|
      generate_callable name, tree
    end.join("\n").split("\n").map do |line|
      level -= 1 if line[-1] == "}" || line[0] == "}" 
      newline = ("    " * level) + line
      level += 1 if line[-1] == "{"
      newline
    end.join("\n")
  end

  def generate_callable(name, statements)
    active_variables = []
    new_variables = []
    code = generate_block statements, active_variables: active_variables, new_variables: new_variables
    (active_variables - new_variables).uniq.each do |var|
      code.unshift "object ir_cmp_#{var} = local(\"#{var}\");"
    end
    new_variables.uniq.each do |var|
      code.unshift "object ir_cmp_#{var} = NULL;"
    end
    
    code.unshift "object #{name}(struct dict * bindings) {"
    code << "}"
    code.join("\n")
  end

  def generate_block(statements, modified_variables: nil, active_variables: nil, new_variables: nil)
    code = []
    modified_variables ||= []
    new_variables ||= []
    active_variables ||= []
    statements.each do |statement|
      case statement.first
        when :"="
          # Assignment
          var = statement[1]
          code.concat save_vars(modified_variables) if contains_lambda? statement[2]
          val = generate_expression statement[2], active_variables: active_variables
          code << "ir_cmp_#{var} = #{val};"
          unless active_variables.include? var
            active_variables << var
            new_variables << var
          end
          unless modified_variables.include? var
            modified_variables << var
          end
        when :if
          clauses = statement[1]
          code.concat save_vars(modified_variables) if clauses.any? {|cl| contains_lambda? cl[0] }
          code << "if (TRUTHY(#{generate_expression(clauses[0][0], active_variables: active_variables)})) {"
          code << generate_block(clauses[0][1], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables)
          clauses[1..-1].each do |clause|
            if clause[0] == :else
              code << "} else {"
            else
              code << "} else if (TRUTHY(#{generate_expression(clause[0], active_variables: active_variables)})) {"
            end
            code << generate_block(clause[1], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables)
          end
          code << "}"
        when :while
          code.concat save_vars(modified_variables) if contains_lambda? statement[1]
          code << "while (TRUTHY(#{generate_expression(statement[1], active_variables: active_variables)})) {"
          code << generate_block(statement[2], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables)
          code << "}"
        when :unless
          code << "if (FALSY(#{generate_expression(statement[1][0][0], active_variables: active_variables)})) {"
          code << generate_block(statement[1][0][1], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables)
          code << "}"
        when :nofunction
          warn "Not Yet Implemented: nofunction"
        when :nomethod
          warn "Not Yet Implemented: nomethod"
        when :begin
          warn "Not Yet Implemented: exceptions"
        when :set
          warn "Not Yet Implemented: setting of attributes"
        when :return
          code << "return #{generate_expression(statement[1], active_variables: active_variables)};"
      end
    end
    code
  end

  def generate_expression(expr, active_variables: nil)
    active_variables ||= []
    if expr.is_a? Symbol
      # Identifier
      case expr
        when :true
          "TRUE"
        when :false
          "FALSE"
        when :nil
          "NIL"
        else
          if expr.to_s.include? ":"
            # Atom Literal
            "ATOM(\"#{expr.to_s[1..-1]}\")"
          else
            # Variable
            active_variables << expr unless active_variables.include? expr
            "(ir_cmp_#{expr} ? ir_cmp_#{expr} : local(\"#{expr}\"))"
          end
      end
    elsif expr.is_a? Fixnum
      # Number literal
      "FIXNUM(#{expr})"
    elsif expr.is_a? Float
      "IR_FLOAT(#{expr})"
    elsif expr.is_a? String
      # String literal
      "IR_STRING(\"#{expr}\")"
    else
      # Compound expression
      case expr.first
        when :"()"
          # Invocation
          args = expr[2]
          arg_ary = args.reduce("array_new()") { |acc, arg| "array_push(#{acc}, #{generate_expression(arg, active_variables: active_variables)})" }
          "calls(#{generate_expression(expr[1], active_variables: active_variables)}, #{arg_ary})"
        when :"."
          # Attribute Get
          "get_attribute(#{generate_expression(expr[1], active_variables: active_variables)}, ATOM(\"#{expr[2]}\"))"
        when :lambda
          # Annonymous Function
          # [:lambda, [:x, {y: 10}, [:destructure, :z]], "code_name"]
          "FUNCTION(ATOM(\"lambda\"), #{generate_arglist(expr[1], active_variables: active_variables)}, locals, #{expr[2]})"
      end
    end
  end

  def generate_arglist(args, active_variables: nil)
    active_variables ||= []
    args.reverse.reduce("NULL") do |acc, arg|
      val = if arg.is_a? Symbol
              # Standard Argument
              "ATOM(\"#{arg}\"), NULL, 0"
            elsif arg.is_a? Hash
              # Default Value
              "ATOM(\"#{arg.first[0]}\"), #{generate_expression args.first[1], active_variables: active_variables}, 0"
            else
              # Splat
              "ATOM(\"#{arg[1]}\"), NULL, 1"
            end
      "list_cons(#{acc}, argument_new(#{val}))"
    end
  end

  def contains_lambda?(expr)
    if expr.is_a? Symbol
      # Identifier
      false
    elsif expr.kind_of? Numeric
      # Number literal
      false
    elsif expr.is_a? String
      # String literal
      false
    else
      # Compound expression
      case expr.first
        when :"()", :"."
          # Invocation / Attribute Get
          contains_lambda? expr[1]
        when :lambda
          # Annonymous Function
          true
      end
    end
  end

  def save_vars(active_variables)
    active_variables.map do |var|
      "set_local(\"#{var}\", ir_cmp_#{var});"
    end
  end
end
