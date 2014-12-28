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
  function invoke(fun)
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
    # foo is defined for instances of Child
    nofunction foo
    # now, foo is no longer defined for instances of Child
  end
```

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

Iridium is a dynamically typed language, where most language constructs are objects.

The primary types in Iridium are:

  * Numeric
    * Integer
    * Float (double precision)
    * Rational
    * Complex
  * String
  * Collection
    * List (a linked list structure)
    * Tuple (an array structure)
    * Dictionary (a dictionary key, value structure)
  * Atom
  
Other (more technical) types are:
  
  * Class
  * Module
  * Function
  * Boolean (`true` and `false`)
  * `nil`

## Collections

### Lists

Lists are defined as a literal with square brackets. As a linked list structure, they know about their head and tail, and can be composed into other lists.

```
  my_list = [1, 2, 3]
  puts(my_list)
  puts(my_list.head())
  puts(my_list.tail())
  a = my_list.cons(5)
  puts(a)
  puts(my_list)
```

Running the above code would result in:

```
  [1, 2, 3]
  1
  [2, 3]
  [5, 1, 2, 3]
  [1, 2, 3]
```

Note how the `cons` method does not mutate the receiver, it returns a new list.

Lists can be indexed using `fetch` and `put`, or using square brackets
.
```
  [1, 2, 3].fetch(0) # => 1
  [1, 2, 4].put(2, 3) # => [1, 2, 3] (and mutates the receiver)
  [1, 2, 3][0] # => 1 (same as first example)
  [1, 2, 3][3] # => nil
  [1, 2, 3].fetch(3) => Error!
```

Typically, tuples are used for data where the index is of concern (it is also much more efficient with tuples).

Since Iridium does not contain a for loop, the `each` method is used to iterate over lists

```
  [1, 2, 3].each -> (element)
    print(element)
  end
```

### Dictionaries

Dictionaries are defined using `%{}` syntax. They act as key-value pairs, and values can be accessed using the `fetch` and `put` methods, or using square brackets.

```
  %{ :a => 5, "boo" => bar, MyClass => -> return 17 end }
```

If the keys are atoms (highly recommended!), a shorthand can be used when defining:

```
  %{ a: 5, b: 6 }
```

Dictionaries can also be iterated over with `each`

```
  %{ :a => 5, :b => 7 }.each -> (key, value)
    # ...
  end
```

### Tuples

Tuples are an array-type structure defined with curly braces `{}`

Since they are an array type, indexing is O(1), which is not true for list structures. Indexing is done the same as with lists.

```
  my_tuple = {1, "a", :c}
  my_tuple[0] # => 1
  my_tuple[1] = "b" # => "b"
  my_tuple # => {1, "b", :c}
```

## Atoms

A simple atom type (known as symbols in some other languages), these elements are identifiers preceded by a colon. They compare by identity, and all atoms of the same name _are the same atom_. As a result, they are often used as keys in dictionaries.

```
  my_atom = :my_atom
  my_atom === :my_atom # => true
```