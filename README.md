iridium
=======

C based language with a simple syntax

## Syntax

### Functions

Functions are easy to define in Iridium. Simple declare them with `function`

```
  function my_func
    # do stuff here...
  end
```

Functions can, of course, accept arguments, which are included an a parenthesized list.

```
  function my_func(arg_1, arg_2, arg_3)
    # do stuff here...
  end
```

The function (with no args) can then be invoked by calling `my_func()`. If it has arguments, they are passed in the parentheiszed list.

Functions can also accept optional arguments by placing `=` between the variable name and default value.

```
  function f(arg_1, arg2 = 3)
    # do stuff here
  end
```

Optional arguments must appear after required arguments.

```
  # Allowed

  function f1(a, b=1)
    # ...
  end

  # Not Allowed

  function f2(a=1, b)
    # ...
  end

  function f3(a, b=2, c)
    # ...
  end

  function f4(a=1, b, c=2)
    # ...
  end
```

Arrays can also be converted into arguments using the `*` operator. This is allowed once per function definition, but can be used multiple times during function invocations. This allows for functions with arbitrary artiy.

```
  function x(a, * args, b = 2, c = 4)
    return [a, args, b, c]
  end

  x(1)
    # => [1, [], 2, 4]
  x(1, 3)
    # => [1, [], 3, 4]
  x(1, 1, 3)
    # => [1, [], 1, 3]
  x(1, 1, 3, 5)
    # => [1, [1], 3, 5]
  x(*[1, 1, 3, 5]) # <=> x(1, 1, 3, 5)
    # => [1, [1], 3, 5]
  x(*[1], 1, *[3, 5]) # <=> x(1, 1, 3, 5)
    # => [1, [1], 3, 5]
  x(1, 1, 2, 3, 5, 8)
    # => [1, [1, 2, 3], 5, 8]
```

#### Annonymous Functions

Functions also come in an unnamed form, which have a few special properties.

```
  my_func = -> (arg_1, arg_2)
    # do stuff here...
  end
```

The variable `my_func` could then be invoked with `my_func(a, b)` (for example).

Unlike named functions, which have their own, isolated scope, annonymous functions can act as closures.

```
  a = 5

  function named
    return a # Not defined
  end
  annonymous = ->
    return a
  end

  named() # => Exception!
  annonymous() # => 5

  a = "foo"
  annonymous() # => 5
```

Since annonymous functions are so useful, there is a special syntax for passing them to functions (though the normal syntax works as well)

```
  function invoke(fun)
    return fun()
  end

  invoke(-> return 5 end) # => 5

  invoke(->
    return 5
  end) # => 5

  # Special Syntax

  invoke ->
    return 5
  end # => 5

  # Special Syntax with arguments

  function add_n(n, fun)
    return n + fun()
  end

  add_n(5) ->
    return 12
  end # => 17
```

Note that annonymous functions cannot recurse without first being bound to a variable.

##### A Note on Closures

As stated previously, annonymous functions retain the bindings present in the scope in which they are created. A notable exception is `self`, which, if the function is made an attribute of an object (turned into a method), is the receiver of the call.

```
self # => #<Object(main)>
me = self

fun_1 = ->
  return self
end

fun_2 = ->
  return me
end

class Foo
  method invoke(fun)
    # self is an instance of the Foo class
    fun()
  end
end

foo = Foo.new() # => #<Foo:0x00000101a688b>
foo.fun_1 = fun_1
foo.fun_2 = fun_2

fun_1() # => #<Object(main)>

fun_2() # => #<Object(main)>

foo.invoke(fun_1) # => #<Object(main)>

foo.invoke(fun_2) # => #<Object(main)>

foo.fun_1() # => #<Foo:0x00000101a688b>

foo.fun_2() # => #<Object(main)>
```

### Classes

Classes in Iridium are defined with the `class` keyword.

```
  class MyClass
    # methods go here
  end
```

Classes can also inherit from other classes and obtain the methods of their superclass.

```
  class MyOtherClass < MyClass
    # inherits from MyClass
    # methods go here
  end
```

Sometimes, a module or superclass defines a method that isn't wanted. This can be fixed by the `nofunction` keyword. Think of it as the opposite of the `function` keyword.

```
  class Base
    function foo
      return :bar
    end
  end

  class Child < Base
    # foo is defined for Child
    nofunction foo
    # now, foo is no longer defined for Child
  end
```

#### A Note on Methods

Like many languages, Iridium distinguishes between instance methods and class methods (this actually generalizes to attributes in general, but is primarily visible through methods). For example, if you define a class A, any methods defined with the `function` keyword become avaliable to A, but not to its instances.

```
class A
  function foo
    return 5
  end
end

A.foo() # => 5
A.new().foo() # Exception!
```

To define methods on the instances of objects, use the `method` keyword.

```
class B
  method bar
    return 6
  end
end

B.bar() # Exception!
B.new().bar() # => 6
```

In addition, a `nomethod` keyword is defined with the same behavior as the `nofunction` keyword.

### Modules

Module allow the namespacing of functions, and the composition of objects by including or extending them.

#### Using Modules as a Namespace

Functions can be namespaced to a module by defining them within the module.

```
  module MyModule
    function my_namespaced_func
      return 5
    end
  end

  my_namespaced_func() # Exception!
  MyModule.my_namespaced_func() # => 5
```

Modules and classes can be namespaced in a similar manner. The method of accessing them is like `MyModule.MyClass`. Accessing in this manner is required even when within the scope of `MyModule`.

#### Composing objects from Modules

Modules can be included by classes, which grant their attributes to the classes instances. Alternatively, Modules can be extended, which provides their attributes to the extending object directly. While only Classes and Modules can `include` Modules, any object can `extend` modules.

```
  class MyClass
    self.include(MyModule)
    # gets attributes from MyModule as instance attributes
    self.extend(MyModule)
    # gets attributes from MyModule
  end
```

## Types

Iridium is a dynamically typed language, where all language constructs are objects.

The primary types in Iridium are:

  * Numeric
    * Integer
    * Float (double precision)
    * Rational
    * Complex
  * String
  * Collection
    * Array
    * Dictionary (a dictionary key, value structure)
  * Atom
  
Other (more technical) types are:
  
  * Class
  * Module
  * Function
  * Boolean (`true` and `false`)
  * `nil`

## Collections


### Dictionaries

Dictionaries are defined using `{}` syntax. They act as key-value pairs, and values can be accessed using the `fetch` and `put` methods, or using square brackets.

```
  { :a => 5, "boo" => bar, MyClass => -> return 17 end }
```

If the keys are atoms (highly recommended!), a shorthand can be used when defining:

```
  { a: 5, b: 6 }
```

Dictionaries can also be iterated over with `each`

```
  { :a => 5, :b => 7 }.each -> (key, value)
    # ...
  end
```

### Arrays

Arrays are a collection structure defined with square braces `[]`

Since they are an array type, indexing is O(1), which is not true for list structures. Indexing is done with the indexing operator.

```
  my_array = [1, "a", :c]
  my_array[0] # => 1
  my_array[1] = "b" # => "b"
  my_array # => [1, "b", :c]
```

## Atoms

A simple atom type (known as symbols in some other languages), these elements are identifiers preceded by a colon. They compare by identity, and all atoms of the same name _are the same atom_. As a result, they are often used as keys in dictionaries.

```
  my_atom = :my_atom
  my_atom === :my_atom # => true
```

## Exceptions

Exceptions can be handled using `begin` blocks.

```
  begin
    # Code to have exceptions handled within
  rescue SomeException
    # Run when code in the `begin` block raises SomeException
  rescue AnotherException, Exception3 => e
    # Rescues AnotherException and Exception3, binding the exception object to the variable `e`
  else
    # Run if no exception is raised
  ensure
    # Run regardless of whether an exception has occurred
  end
```

Raising exceptions is done with the `raise` keyword.