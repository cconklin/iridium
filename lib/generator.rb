require_relative 'translator'

class Generator
  def initialize(callables, tree)
    @callables = callables
    @tree = tree
    @constants = []
  end

  def generate
    level = 0
    code = [generate_includes, [@constants], generate_prototypes, generate_callables, generate_main]
    gencode = code.join("\n").split("\n").map do |line|
      level -= 1 if line[-1] == "}" || line[0] == "}" 
      newline = ("    " * level) + line
      level += 1 if line[-1] == "{"
      newline
    end.join("\n")
    gencode
  end

  def generate_includes
    %w[src/iridium].map do |header|
      header_path = File.absolute_path File.join(File.dirname(__FILE__), "..", header)
      "#include \"#{header_path}.h\""
    end.join("\n")
  end

  def generate_prototypes
    @callables.keys.map do |name|
      "object #{name}(struct dict * locals);"
    end.join("\n")
  end

  def generate_main
    code = []
    new_variables = []
    active_variables = []
    modified_variables = []
    literals = {}
    builtin_constants = %i[Object Class Atom Function List Tuple Dictionary Fixnum Float String Module NilClass]
    open_constants = %i[Object Class Atom Function List Tuple Dictionary Fixnum Float String Module NilClass]
    
    # Ensure that self is put in any closures
    modified_variables << "self"

    generate_main_block code, @tree, new_variables: new_variables,
                                     active_variables: active_variables,
                                     modified_variables: modified_variables,
                                     literals: literals,
                                     open_constants: open_constants
    
    (active_variables - new_variables - [:self]).uniq.each do |var|
      code.unshift "object ir_cmp_#{var} = local(\"#{var}\");" unless var[0] == var[0].upcase
    end
    (new_variables - [:self]).uniq.each do |var|
      code.unshift "object ir_cmp_#{var} = NULL;" unless var[0] == var[0].upcase
    end

    literals.each do |name, value|
      code.unshift "object #{name} = #{value};"
    end

    (open_constants - builtin_constants).each do |constant|
      @constants.unshift "object ir_cmp_#{constant} = NULL;"
    end

    # Set up the initial context
    code.unshift "array_push(self_stack, ir_cmp_self);"
    code.unshift "dict_set(locals, ATOM(\"self\"), ir_cmp_self);"
    code.unshift "object ir_cmp_self = ir_main;"
    code.unshift "struct array * self_stack = array_new();"
    code.unshift "object ir_main = IR_MAIN_OBJECT();"
    code.unshift "struct dict * locals = dict_new(ObjectHashsize);"
    code.unshift "void ir_user_main() {"
    
    code << "}"
    code.join("\n") 
  end

  def push_self(code, new_self)
    code << "ir_cmp_self = #{new_self};"
    code << "dict_set(locals, ATOM(\"self\"), ir_cmp_self);"
    code << "array_push(self_stack, ir_cmp_self);"
  end

  def pop_self(code, method: :<<)
    code << "ir_cmp_self = array_pop(self_stack);"
    code << "dict_set(locals, ATOM(\"self\"), ir_cmp_self);"
  end

  def generate_main_block(code, tree, new_variables:, active_variables:, modified_variables:, literals:, open_constants:)
     tree.each do |node|
      if node.respond_to? :each
        case node.first
        when :module
          name = node[1]
          unless open_constants.include? name
            open_constants << name
            code << "ir_cmp_#{name} = invoke(ir_cmp_Module, \"new\", array_push(array_new(), IR_STRING(\"#{name}\")));"
          end
          push_self code, "ir_cmp_#{name}"
          generate_main_block code, node[2], new_variables: new_variables,
                                             active_variables: active_variables,
                                             modified_variables: modified_variables,
                                             literals: literals,
                                             open_constants: open_constants
          pop_self code
        when :class
          name = node[1]
          superclass = node[2] || :Object
          unless open_constants.include? name
            open_constants << name
            code << "ir_cmp_#{name} = invoke(ir_cmp_Class, \"new\", array_push(array_push(array_new(), IR_STRING(\"#{name}\")), ir_cmp_#{superclass}));"
          end
          push_self code, "ir_cmp_#{name}"
          generate_main_block code, node[3], new_variables: new_variables,
                                             active_variables: active_variables,
                                             modified_variables: modified_variables,
                                             literals: literals,
                                             open_constants: open_constants
          pop_self code
        when :function
          new_variables << node[1]
          code << "set_attribute(ir_cmp_self, ATOM(\"#{node[1]}\"), PUBLIC,"
          code << "  FUNCTION(ATOM(\"#{node[1]}\"), #{generate_arglist(node[2], active_variables: active_variables, literals: literals)}, dict_new(ObjectHashsize), #{node[3]} ));"
        when :method
          new_variables << node[1]
          code << "set_instance_attribute(ir_cmp_self, ATOM(\"#{node[1]}\"), PUBLIC,"
          code << "  FUNCTION(ATOM(\"#{node[1]}\"), #{generate_arglist(node[2], active_variables: active_variables, literals: literals)}, dict_new(ObjectHashsize), #{node[3]} ));"
        else
          # Arbitrary statement
          generate_statement code, node, modified_variables: modified_variables,
                                         active_variables: active_variables,
                                         new_variables: new_variables,
                                         literals: literals
        end
      else
        # Arbitrary statement
        generate_statement code, node, modified_variables: modified_variables,
                                       active_variables: active_variables,
                                       new_variables: new_variables,
                                       literals: literals
      end
    end
   
  end

  def generate_callables
    @callables.flat_map do |name, tree|
      generate_callable name, tree
    end
  end

  def generate_callable(name, statements)
    active_variables = []
    new_variables = []
    literals = {}
    code = generate_block statements, active_variables: active_variables, new_variables: new_variables, literals: literals
    (active_variables - new_variables).uniq.each do |var|
      code.unshift "object ir_cmp_#{var} = local(\"#{var}\");" unless var[0] == var[0].upcase
    end
    new_variables.uniq.each do |var|
      code.unshift "object ir_cmp_#{var} = NULL;" unless var[0] == var[0].upcase
    end
    literals.each do |name, value|
      code.unshift "object #{name} = #{value};"
    end
    code.unshift "object #{name}(struct dict * locals) {"
    code << "return NIL;"
    code << "}"
    code.join("\n")
  end

  def generate_block(statements, modified_variables: nil, active_variables: nil, new_variables: nil, literals: nil)
    code = []
    modified_variables ||= []
    new_variables ||= []
    active_variables ||= []
    literals ||= {}
    statements.each do |statement|
      generate_statement code, statement, modified_variables: modified_variables,
                                          active_variables: active_variables,
                                          new_variables: new_variables,
                                          literals: literals
    end
    code
  end

  def generate_statement(code, statement, modified_variables:, active_variables:, new_variables:, literals:)
    case statement.first
      when :"="
        # Assignment
        var = statement[1]
        code.concat save_vars(modified_variables) if contains_lambda? statement[2]
        val = generate_expression statement[2], active_variables: active_variables, literals: literals
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
        code << "if (TRUTHY(#{generate_expression(clauses[0][0], active_variables: active_variables, literals: literals)})) {"
        code << generate_block(clauses[0][1], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals)
        clauses[1..-1].each do |clause|
          if clause[0] == :else
            code << "} else {"
          else
            code << "} else if (TRUTHY(#{generate_expression(clause[0], active_variables: active_variables, literals: literals)})) {"
          end
          code << generate_block(clause[1], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals)
        end
        code << "}"
      when :while
        code.concat save_vars(modified_variables) if contains_lambda? statement[1]
        code << "while (TRUTHY(#{generate_expression(statement[1], active_variables: active_variables, literals: literals)})) {"
        code << generate_block(statement[2], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals)
        code << "}"
      when :unless
        code << "if (FALSY(#{generate_expression(statement[1][0][0], active_variables: active_variables, literals: literals)})) {"
        code << generate_block(statement[1][0][1],
                               modified_variables: modified_variables,
                               active_variables: active_variables,
                               new_variables: new_variables,
                               literals: literals)
        code << "}"
      when :nofunction
        code << "no_attribute(#{generate_expression(:self, active_variables: active_variables, literals: literals)},"
        code << "             ATOM(\"#{statement[1]}\");"
      when :nomethod
        code << "no_instance_attribute(#{generate_expression(:self, active_variables: active_variables, literals: literals)},"
        code << "                      ATOM(\"#{statement[1]}\");"
      when :begin
        warn "Not Yet Implemented: exceptions"
      when :set
        code << "set_attribute(#{generate_expression(statement[1], active_variables: active_variables, literals: literals)},"
        code << "              ATOM(\"#{statement[2]}\"), PUBLIC, #{generate_expression(statement[3], active_variables: active_variables, literals: literals)});"
      when :return
        code << "return #{generate_expression(statement[1], active_variables: active_variables, literals: literals)};"
      else
        # Some expression
        code << generate_expression(statement, active_variables: active_variables, literals: literals) + ";"
    end
  end

  def generate_expression(expr, active_variables: nil, literals: nil)
    active_variables ||= []
    literals ||= {}
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
      value = "FIXNUM(#{expr})"
      name = "ir_lit_#{Translator.name(value)}"
      literals[name] = value
      name
    elsif expr.is_a? Float
      value = "IR_FLOAT(#{expr})"
      name = "ir_lit_#{Translator.name(value)}"
      literals[name] = value
      name
    elsif expr.is_a? String
      # String literal
      "IR_STRING(\"#{expr}\")"
    else
      # Compound expression
      case expr.first
        when :"()"
          # Invocation
          args = expr[2]
          arg_ary = args.reduce("array_new()") do |acc, arg|
            if arg.respond_to?(:[]) && arg[0] == :destructure
              "destructure(#{acc}, #{generate_expression(arg[1], active_variables: active_variables, literals: literals)})"
            else
              "array_push(#{acc}, #{generate_expression(arg, active_variables: active_variables, literals: literals)})"
            end
          end
          "calls(#{generate_expression(expr[1], active_variables: active_variables, literals: literals)}, #{arg_ary})"
        when :"."
          # Attribute Get
          "invoke(#{generate_expression(expr[1], active_variables: active_variables, literals: literals)}, \"__get__\", array_push(array_new(), ATOM(\"#{expr[2]}\")))"
        when :lambda
          # Annonymous Function
          # [:lambda, [:x, {y: 10}, [:destructure, :z]], "code_name"]
          "FUNCTION(ATOM(\"lambda\"), #{generate_arglist(expr[1], active_variables: active_variables, literals: literals)}, locals, #{expr[2]})"
        when :list
          arg_ary = expr[1].reduce("array_new()") do |acc, arg|
            if arg.respond_to?(:[]) && arg[0] == :destructure
              "destructure(#{acc}, #{generate_expression(arg[1], active_variables: active_variables, literals: literals)})"
            else
              "array_push(#{acc}, #{generate_expression(arg, active_variables: active_variables, literals: literals)})"
            end
          end
          "invoke(ir_cmp_List, \"new\", #{arg_ary})"
        when :tuple
          arg_ary = expr[1].reduce("array_new()") do |acc, arg|
            if arg.respond_to?(:[]) && arg[0] == :destructure
              "destructure(#{acc}, #{generate_expression(arg[1], active_variables: active_variables, literals: literals)})"
            else
              "array_push(#{acc}, #{generate_expression(arg, active_variables: active_variables, literals: literals)})"
            end
          end
          "invoke(ir_cmp_Tuple, \"new\", #{arg_ary})"
        when :dictionary
          warn "Not Yet Implemented: dictionary literals"
        when :===
          "(#{generate_expression(expr[1], active_variables: active_variables, literals: literals)} == #{generate_expression(expr[2], active_variables: active_variables, literals: literals)} ? ir_cmp_true : ir_cmp_false)"
      end
    end
  end

  def generate_arglist(args, active_variables: nil, literals: nil)
    active_variables ||= []
    literals ||= {}
    args.reverse.reduce("NULL") do |acc, arg|
      val = if arg.is_a? Symbol
              # Standard Argument
              "ATOM(\"#{arg}\"), NULL, 0"
            elsif arg.is_a? Hash
              # Default Value
              "ATOM(\"#{arg.first[0]}\"), #{generate_expression arg.first[1], active_variables: active_variables, literals: literals}, 0"
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
