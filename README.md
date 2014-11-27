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

The functions can then be invoked by calling `my_func()`.

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
  %{ :a => 5, :b => 7 }.each -> (pair)
    key = pair[0]
    value = pair[1]
    # ...
  end
```

The pair is currently a tuple, and expansion of the tuple into two separate values will likely be included in the future.

### Tuples

Tuples are an array-type structure defined with curly braces `{}`

Since they are an array-type, indexing is O(1), which is not true for list structures. Indexing is done the same as with lists.

## Atoms

A simple atom type (known as symbols in some other languages), these elements are identifiers preceded by a colon. They compare by identity, and all atoms of the same name _are the same atom_. As a result, they are often used as keys in dictionaries.