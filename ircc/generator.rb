require_relative 'translator'

class Generator
  def initialize(callables, tree, main_name, link, user_atoms)
    @callables = callables
    @tree = tree
    @constants = []
    @self_stack = []
    @main_fn_name = main_name
    @link = link
    @atoms = []
    @user_atoms = user_atoms
  end

  def generate
    level = 0
    code = [generate_includes, [@constants], generate_prototypes, generate_callables, generate_main]
    code.insert(1, generate_atoms)
    gencode = code.join("\n").split("\n").map do |line|
      level -= 1 if line[-1] == "}" || line[0] == "}" 
      newline = ("    " * level) + line
      level += 1 if line[-1] == "{"
      newline
    end.join("\n")
    gencode
  end

  def iridium_path(path)
    File.absolute_path File.join(File.dirname(__FILE__), "..", path)
  end

  def generate_includes
    %w[iridium/include/ir_init iridium/include/atoms].map do |header|
      "#include \"#{iridium_path header}.h\""
    end.join("\n")
  end

  def generate_prototypes
    @callables.keys.map do |name|
      "object #{name}(struct IridiumContext * context, struct dict * locals);"
    end.join("\n")
  end

  def new_atoms
    @atoms.map(&:to_s).uniq - predefined_atoms
  end

  def predefined_atoms
    predefined = File.open(iridium_path "iridium/include/atoms.txt").read.split("\n")
    if @user_atoms
      user = File.open(iridium_path "iridium/include/user_atoms.txt").read.split("\n")
      return (predefined + user).uniq
    end
    predefined.uniq
  end

  def generate_atoms
    extern_atoms = predefined_atoms.map do |a|
      "extern object #{ir_atom_name a};"
    end
    atoms = new_atoms.uniq.map do |a|
      [
        "static struct IridiumObject _#{ir_atom_name a};",
        "object #{ir_atom_name a} = &_#{ir_atom_name a};",
      ].join "\n"
    end
    definitions = new_atoms.uniq.map do |atom_name|
      name = ir_atom_name atom_name
      [
        "#{name}->magic = MAGIC;",
        "#{name}->class = CLASS(Atom);",
        "#{name}->attributes = dict_new(ObjectHashsize);",
        "#{name}->instance_attributes = dict_new(ObjectHashsize);",
        "#{name}->internal_attributes = dict_new(ObjectHashsize);",
        "add_atom(\"#{atom_name}\", #{name});",
        "internal_set_attribute(#{name}, _IR_ATOM_string, \"#{atom_name}\");",
      ].join("\n")
    end.join("\n")
    func = @user_atoms ? "IR_USER_ATOM_INIT" : "__IR_USER_ATOM_INIT"
    [
      "void __IR_USER_ATOM_INIT(void);",
      extern_atoms,
      atoms,
      "void #{func}(void) {",
      definitions,
      @user_atoms ? "__IR_USER_ATOM_INIT();" : nil,
      "}",
    ].compact.join "\n"
  end

  def atom(name)
    @atoms << name
    ir_atom_name name
  end

  def ir_atom_name(name)
    if name[-1] == "?"
      "_IR_Q_ATOM_#{name[0...-1]}"
    elsif name[-1] == "!"
      "_IR_B_ATOM_#{name[0...-1]}"
    else
      "_IR_ATOM_#{name}"
    end
  end

  def generate_main
    code = []
    new_variables = []
    active_variables = []
    modified_variables = []
    literals = {}
    exception_handlers = []
    open_constants = %i[Object Class Atom Function Array Dictionary Integer Float String Module NilClass
                        File FileNotFoundError Exception IOError AttributeError TypeError Queue Queue.Empty
                        ArgumentError Thread Regex]
    # Ensure that self is put in any closures
    modified_variables << "self"

    generate_main_block code, @tree, new_variables: new_variables,
                                     active_variables: active_variables,
                                     modified_variables: modified_variables,
                                     literals: literals,
                                     open_constants: open_constants, 
                                     exception_handlers: exception_handlers
    
    (active_variables - new_variables - [:self]).uniq.each do |var|
      code.unshift "object #{variable_name(var)} = NULL;" unless open_constants.include? var
    end
    (new_variables - [:self]).uniq.each do |var|
      code.unshift "object #{variable_name(var)} = NULL;" unless open_constants.include? var
    end

    literals.each do |name, value|
      code.unshift "object #{name} = #{value};"
    end

    exception_handlers.each do |handler|
      code.unshift "exception_frame #{handler};"
    end

    # Set up the initial context
    code.unshift "array_push(self_stack, ir_cmp_self);"
    code.unshift "dict_set(locals, #{atom "self"}, ir_cmp_self);"
    code.unshift "object ir_cmp_self = ir_main;"
    code.unshift "struct array * self_stack = array_new();"
    code.unshift "ir_context_stack = array_new();"
    code.unshift "ir_context = ir_main;"
    code.unshift "object ir_main = IR_MAIN_OBJECT(context);"
    code.unshift "struct dict * locals = dict_new(ObjectHashsize);"
    code.unshift "int _handler_count = 0;"
    code.unshift "object #{@main_fn_name}(struct IridiumContext * context) {"

    code << "return NIL;"
    code << "}"
    if @link
      code << "int main(int argc, const char ** argv) {"
      code << "GC_INIT();"
      # TODO ARGV
      code << "struct IridiumContext context;"
      code << "IR_EARLY_INIT(&context);"
      code << "IR_USER_ATOM_INIT();"
      code << "IR_INIT(&context);"
      # FIXME only catch children of exception / string?
      code << "struct list * main_exceptions = list_new(EXCEPTION(CLASS(Object), 1));"
      code << "exception_frame e = ExceptionHandler(&context, main_exceptions, 0, 0, 0);"
      code << "switch (setjmp(e->env)) {"
      code << "case 0:"
      code << "#{@main_fn_name}(&context);"
      code << "return 0;"
      code << "case 1:"
      code << "// any uncaught exception"
      code << 'display_stacktrace(&context, context._raised);'
      code << "return 1;"
      code << "}"
      code << "}"
    end
    code.join("\n") 
  end

  def last_constant
    @self_stack[1..-1].reduce "lookup_constant(context, #{atom @self_stack[0]})" do |acc, n|
      "get_attribute(#{acc}, #{atom n}, PUBLIC)"
    end
  end

  def get_constant(name)
    if @self_stack.empty?
      "lookup_constant(context, #{atom name})"
    else
      "get_attribute(#{last_constant}, #{atom name}, PUBLIC)"
    end
  end

  def push_self(code, name)
    new_self = get_constant(name)
    @self_stack << name
    code << "array_push(self_stack, ir_cmp_self);"
    code << "array_push(ir_context_stack, ir_context);"
    code << "ir_context = #{new_self};"
    code << "ir_cmp_self = #{new_self};"
    code << "dict_set(locals, #{atom "self"}, ir_cmp_self);"
  end

  def pop_self(code)
    @self_stack.pop
    code << "ir_context = array_pop(ir_context_stack);"
    code << "ir_cmp_self = array_pop(self_stack);"
    code << "dict_set(locals, #{atom "self"}, ir_cmp_self);"
  end

  def generate_main_block(code, tree, new_variables:, active_variables:, modified_variables:, literals:, open_constants:, exception_handlers:)
     tree.each do |node|
      if node.respond_to? :each
        case node.first
        when :module
          name = node[1]
          full_name = (@self_stack + [name]).map(&:to_s).join(".")
          unless open_constants.include? full_name.to_sym
            open_constants << full_name.to_sym
            constant = "invoke(context, ir_cmp_Module, \"new\", array_push(array_new(), IR_STRING(\"#{full_name}\")))"
            if @self_stack.empty?
              # Top level module
              code << "define_constant(#{atom name}, #{constant});"
            else
              # Child of parent
              code << "set_attribute(#{last_constant}, #{atom name}, PUBLIC, #{constant});"
            end
          end
          push_self code, name
          generate_main_block code, node[2], new_variables: new_variables,
                                             active_variables: active_variables,
                                             modified_variables: modified_variables,
                                             literals: literals,
                                             open_constants: open_constants,
                                             exception_handlers: exception_handlers
          pop_self code
        when :class
          name = node[1]
          superclass = node[2] || :Object
          full_name = (@self_stack + [name]).map(&:to_s).join(".")
          unless open_constants.include? full_name.to_sym
            open_constants << full_name.to_sym
            constant = "invoke(context, ir_cmp_Class, \"new\", array_push(array_push(array_new(), IR_STRING(\"#{full_name}\")), #{generate_expression(superclass)}))"
            if @self_stack.empty?
              # Top level module
              code << "define_constant(#{atom name}, #{constant});"
            else
              # Child of parent
              code << "set_attribute(#{last_constant}, #{atom name}, PUBLIC, #{constant});"
            end
          end
          push_self code, name
          generate_main_block code, node[3], new_variables: new_variables,
                                             active_variables: active_variables,
                                             modified_variables: modified_variables,
                                             literals: literals,
                                             open_constants: open_constants,
                                             exception_handlers: exception_handlers
          pop_self code
        when :function
          new_variables << node[1]
          code << "set_attribute(ir_cmp_self, #{atom node[1]}, PUBLIC,"
          code << "  FUNCTION(#{atom node[1]}, #{generate_arglist(node[2], active_variables: active_variables, literals: literals)}, dict_new(ObjectHashsize), #{node[3]} ));"
        when :method
          new_variables << node[1]
          code << "set_instance_attribute(ir_cmp_self, #{atom node[1]}, PUBLIC,"
          code << "  FUNCTION(#{atom node[1]}, #{generate_arglist(node[2], active_variables: active_variables, literals: literals)}, dict_new(ObjectHashsize), #{node[3]} ));"
        else
          # Arbitrary statement
          generate_statement code, node, modified_variables: modified_variables,
                                         active_variables: active_variables,
                                         new_variables: new_variables,
                                         literals: literals,
                                         in_begin: false,
                                         exception_handlers: exception_handlers
        end
      else
        # Arbitrary statement
        generate_statement code, node, modified_variables: modified_variables,
                                       active_variables: active_variables,
                                       new_variables: new_variables,
                                       literals: literals,
                                       in_begin: false,
                                       exception_handlers: exception_handlers
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
    exception_handlers = []
    code = generate_block statements, active_variables: active_variables, new_variables: new_variables, literals: literals, exception_handlers: exception_handlers
    (active_variables - new_variables).uniq.each do |var|
      code.unshift "object #{variable_name(var)} = NULL;" unless var[0] == var[0].upcase
    end
    new_variables.uniq.each do |var|
      code.unshift "object #{variable_name(var)} = NULL;" unless var[0] == var[0].upcase
    end
    literals.each do |name, value|
      code.unshift "object #{name} = #{value};"
    end
    exception_handlers.each do |handler|
      code.unshift "exception_frame #{handler};"
    end
    code.unshift "int _handler_count = 0;"
    code.unshift "object #{name}(struct IridiumContext * context, struct dict * locals) {"
    code << "return NIL;"
    code << "}"
    code.join("\n")
  end

  def generate_block(statements, modified_variables: nil, active_variables: nil, new_variables: nil, literals: nil, in_begin: false, exception_handlers: nil)
    code = []
    modified_variables ||= []
    new_variables ||= []
    active_variables ||= []
    literals ||= {}
    exception_handlers ||= {}
    statements.each do |statement|
      generate_statement code, statement, modified_variables: modified_variables,
                                          active_variables: active_variables,
                                          new_variables: new_variables,
                                          literals: literals,
                                          in_begin: in_begin, 
                                          exception_handlers: exception_handlers
    end
    code
  end

  def variable_name(var)
    if var.to_s.include? "?"
      "ir_qcmp_#{var[0...-1]}"
    elsif var.to_s.include? "!"
      "ir_bcmp_#{var[0...-1]}"
    else
      "ir_cmp_#{var}"
    end
  end

  def generate_statement(code, statement, modified_variables:, active_variables:, new_variables:, literals:, in_begin:, exception_handlers:)
    unless statement.is_a?(Array)
      code << generate_expression(statement, active_variables: active_variables, literals: literals) + ";"
      return
    end
    case statement.first
      when :"="
        # Assignment
        var = statement[1]
        code.concat save_vars(modified_variables) if contains_lambda? statement[2]
        val = generate_expression statement[2], active_variables: active_variables, literals: literals
        if var.to_s[0].match /[[:lower:]]/
          # Lowercase (e.g. foo): just a variable
          code << "#{variable_name(var)} = #{val};"
          unless active_variables.include? var
            active_variables << var
            new_variables << var
          end
          unless modified_variables.include? var
            modified_variables << var
          end
        else
          # Uppercase (e.g. Foo): constant
          if @self_stack.empty?
            # Top level module
            code << "define_constant(#{atom var}, #{val});"
          else
            # Child of parent
            code << "set_attribute(#{last_constant}, #{atom var}, PUBLIC, #{val});"
          end
        end
      when :if
        clauses = statement[1]
        code.concat save_vars(modified_variables) if clauses.any? {|cl| contains_lambda? cl[0] }
        code << "if (TRUTHY(#{generate_expression(clauses[0][0], active_variables: active_variables, literals: literals)})) {"
        code << generate_block(clauses[0][1], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals, exception_handlers: exception_handlers)
        clauses[1..-1].each do |clause|
          if clause[0] == :else
            code << "} else {"
          else
            code << "} else if (TRUTHY(#{generate_expression(clause[0], active_variables: active_variables, literals: literals)})) {"
          end
          code << generate_block(clause[1], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals, exception_handlers: exception_handlers)
        end
        code << "}"
      when :while
        code.concat save_vars(modified_variables) if contains_lambda? statement[1]
        code << "while (TRUTHY(#{generate_expression(statement[1], active_variables: active_variables, literals: literals)})) {"
        code << generate_block(statement[2], modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals, exception_handlers: exception_handlers)
        code << "}"
      when :unless
        code << "if (FALSY(#{generate_expression(statement[1][0][0], active_variables: active_variables, literals: literals)})) {"
        code << generate_block(statement[1][0][1],
                               modified_variables: modified_variables,
                               active_variables: active_variables,
                               new_variables: new_variables,
                               literals: literals,
                               exception_handlers: exception_handlers)
        code << "}"
      when :nofunction
        code << "no_attribute(#{generate_expression(:self, active_variables: active_variables, literals: literals)},"
        code << "             #{atom statement[1]});"
      when :nomethod
        code << "no_instance_attribute(#{generate_expression(:self, active_variables: active_variables, literals: literals)},"
        code << "                      #{atom statement[1]});"
      when :begin
        # [:begin, [[:"=", :x, 5]], {MyException: [:e, [[:"=", :x, 6]]]}, []]
        begin_section = statement[1]
        rescue_sections = statement[2]
        ensure_section = statement[3]
        handler_id = Translator.name(statement)
        handler_var = "ir_exc_#{handler_id}"
        # No else block allowed yet
        exception_list = rescue_sections.keys.map.with_index do |exc, idx|
          idx += 1 # start @ 1 (begin is 0)
          "EXCEPTION(#{generate_expression(exc, active_variables: active_variables, literals: literals)}, #{idx})"
        end
        exception_list = "ARGLIST(#{exception_list.join(',')})"
        exception_handlers << handler_var
        code << "#{handler_var} = ExceptionHandler(context, #{exception_list}, #{ensure_section.empty? ? 0 : 1}, 0, _handler_count++);"
        # Rest goes here
        code << "switch (setjmp(#{handler_var} -> env)) {"
        code << "case 0:"
        code.concat(generate_block(begin_section, modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals, in_begin: true, exception_handlers: exception_handlers))
        code << "END_BEGIN(context, #{handler_var});"
        rescue_sections.each_with_index do |(exc, handler), i|
          exc_idx = i + 1
          exc_variable = handler[0]
          exc_block = handler[1]
          code << "case #{exc_idx}:"
          if exc_variable
            unless active_variables.include? exc_variable
              active_variables << exc_variable
              new_variables << exc_variable
            end
            unless modified_variables.include? exc_variable
              modified_variables << exc_variable
            end
            code << "#{variable_name(exc_variable)} = context->_raised;"
          end
          code.concat(generate_block(exc_block, modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals, in_begin: true, exception_handlers: exception_handlers))
          code << "END_RESCUE(context, #{handler_var});"
        end
        unless ensure_section.empty?
          code << "case ENSURE_JUMP:"
          code.concat(generate_block(ensure_section, modified_variables: modified_variables, active_variables: active_variables, new_variables: new_variables, literals: literals, in_begin: true, exception_handlers: exception_handlers))
          code << "END_ENSURE(context, #{handler_var});"
        end
        code << "case FRAME_RETURN:"
        code << "if (#{handler_var} -> count == 0) {"
        code << "return #{handler_var} -> return_value;"
        code << "} else {"
        code << "return_in_begin_block(context, #{handler_var} -> return_value);"
        code << "}"
        code << "break;"
        code << "}"
        code << "_handler_count--;"
      when :set
        code << "set_attribute(#{generate_expression(statement[1], active_variables: active_variables, literals: literals)},"
        code << "              #{atom statement[2]}, PUBLIC, #{generate_expression(statement[3], active_variables: active_variables, literals: literals)});"
      when :insert
        # [:insert, :x, [5], [:[], :y, [[:+, 4, 5]]]]
        arg_ary = generate_argarray(statement[2] + [statement[3]], active_variables: active_variables, literals: literals)
        code << "invoke(context, #{generate_expression(statement[1], active_variables: active_variables, literals: literals)}, \"__set_index__\", #{arg_ary});"
      when :return
        if in_begin
          code << "return_in_begin_block(context, #{generate_expression(statement[1], active_variables: active_variables, literals: literals)});"
        else
          code << "return #{generate_expression(statement[1], active_variables: active_variables, literals: literals)};"
        end
      else
        # Some expression
        code.concat save_vars(modified_variables) if contains_lambda? statement
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
          "ir_cmp_true"
        when :false
          "ir_cmp_false"
        when :nil
          "NIL"
        else
          if expr.to_s.include? ":"
            # Atom Literal
            "#{atom expr.to_s[1..-1]}"
          elsif expr.to_s.match /^[^a-zA-Z]*[A-Z].*$/
            # First letter is uppercase -> constant
            "lookup_constant(context, #{atom expr})"
          else
            # Variable
            active_variables << expr unless active_variables.include? expr
            "(#{variable_name(expr)} ? #{variable_name(expr)} : _local(context, locals, #{atom expr}))"
          end
      end
    elsif expr.is_a? Integer
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
          arg_ary = generate_argarray(args, active_variables: active_variables, literals: literals)
          "calls(context, #{generate_expression(expr[1], active_variables: active_variables, literals: literals)}, #{arg_ary})"
        when :"[]"
          # Indexing
          # [:[], :y, [[:+, 4, 5]]]
          arg_ary = generate_argarray(expr[2], active_variables: active_variables, literals: literals)
          "invoke(context, #{generate_expression(expr[1], active_variables: active_variables, literals: literals)}, \"__get_index__\", #{arg_ary})"
        when :"."
          # Attribute Get
          "invoke(context, #{generate_expression(expr[1], active_variables: active_variables, literals: literals)}, \"__get__\", array_push(array_new(), #{atom expr[2]}))"
        when :lambda
          # Annonymous Function
          # [:lambda, [:x, {y: 10}, [:destructure, :z]], "code_name"]
          "FUNCTION(#{atom "lambda"}, #{generate_arglist(expr[1], active_variables: active_variables, literals: literals)}, locals, #{expr[2]})"
        when :array
          arg_ary = generate_argarray(expr[1], active_variables: active_variables, literals: literals)
          "invoke(context, ir_cmp_Array, \"new\", #{arg_ary})"
        when :dictionary
          # [:dictionary, {:":foo" => [:+, 2, 3], "baz" => :X, :v => 3}]
          dict_lst = generate_expression([:array, expr[1].map {|k, v| [:array, [k, v]]}], active_variables: active_variables, literals: literals)
          "invoke(context, ir_cmp_Dictionary, \"new\", array_push(array_new(), #{dict_lst}))"
        when :===
          "(#{generate_expression(expr[1], active_variables: active_variables, literals: literals)} == #{generate_expression(expr[2], active_variables: active_variables, literals: literals)} ? ir_cmp_true : ir_cmp_false)"
        when :regex
          "(#{generate_expression([:"()", [:".", :Regex, :new], [expr[1]]])})"
      end
    end
  end

  def generate_argarray(args, active_variables:, literals:)
    args.reduce("array_new()") do |acc, arg|
      if arg.respond_to?(:[]) && arg[0] == :destructure
        "destructure(context, #{acc}, #{generate_expression(arg[1], active_variables: active_variables, literals: literals)})"
      else
        "array_push(#{acc}, #{generate_expression(arg, active_variables: active_variables, literals: literals)})"
      end
    end
  end

  def generate_arglist(args, active_variables: nil, literals: nil)
    active_variables ||= []
    literals ||= {}
    args.reverse.reduce("NULL") do |acc, arg|
      val = if arg.is_a? Symbol
              # Standard Argument
              "#{atom arg}, NULL, 0"
            elsif arg.is_a? Hash
              # Default Value
              "#{atom arg.first[0]}, #{generate_expression arg.first[1], active_variables: active_variables, literals: literals}, 0"
            else
              # Splat
              "#{atom arg[1]}, NULL, 1"
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
        when :"."
          # Attribute Get
          contains_lambda? expr[1]
        when :"()"
          # invocation
          contains_lambda?(expr[1]) or expr[2].any? do |arg|
            contains_lambda? arg
          end
        when :lambda
          # Annonymous Function
          true
      end
    end
  end

  def save_vars(active_variables)
    active_variables.map do |var|
      "_set_local(locals, #{atom var}, #{variable_name(var)});"
    end
  end
end
