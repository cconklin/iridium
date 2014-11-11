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

Classes in Iridium are defined with the `class` keyword. Classes must declare which attributes that they have.

```
  class MyClass
    attributes :attr_1, :attr_2
    # methods go here
  end
```

Classes can also inherit from other classes, and obtain, without the need for declaration, the attributes of their superclass.

```
  class MyOtherClass < MyClass
    # inherits from MyClass
    attributes :attr_3
    # has the attributes :attr_1, :attr_2, and :attr_3 defined
    # they can be accessed by self.attr_1 or by @attr_1 (for example)
    private_attributes :attr_4
    # :attr_4 is only avaliable in the scope of self as @attr_4 
    # methods go here
  end
```

Attributes can be accessed internally via the `@` sigil, or publicly without. All attributes are public unless declared with `private_attributes` (that is, one could call `my_obj.some_attribute` from anywhere if `some_attribute` is public; if it is private, it is only visible within the methods of `my_obj`).

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
  