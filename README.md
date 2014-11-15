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
  