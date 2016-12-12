module Iridium
  class Block < Treetop::Runtime::SyntaxNode
    def content
      elements.map(&:content)
    end
  end
  
  class Statement < Treetop::Runtime::SyntaxNode
    def content
      elements[0].content
    end
  end

  class Require < Treetop::Runtime::SyntaxNode
    def content
      [:require, elements[0].content]
    end
  end

  class IntegerLiteral < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_i
    end
  end
  
  class FloatLiteral < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_f
    end
  end
  
  class StringLiteral < Treetop::Runtime::SyntaxNode
    def content
      text_value[1...-1]
    end
  end
  
  class AtomLiteral < Treetop::Runtime::SyntaxNode
    def content
      self.text_value.to_sym # :foo => :":foo"
    end
  end
  
  class ArrayLiteral < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 0 # empty list
        [:array, []]
      else
        [:array, elements[0].content]
      end
    end
  end
  
  class DictionaryLiteral < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 0 # Empty dictionary
        [:dictionary, {}]
      else
        [:dictionary, Hash[*elements[0].content]]
      end
    end
  end

  class DictionaryContent < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 2 # dictionary with one element
        [elements[0].content, elements[1].content]
      else
        [elements[0].content, elements[1].content, *elements[2].content]
      end
    end
  end

  class DictionaryKey < Treetop::Runtime::SyntaxNode
    def content
      ":#{elements[0].content}".to_sym # treat %{ foo: bar } as %{ :foo => bar }
    end
  end

  class ListContent < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 1 # list with one element
        [elements[0].content]
      else
        [elements[0].content, *elements[1].content]
      end
    end
  end
  
  class Identifier < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_sym
    end
  end
  
  class OperatorExpression < Treetop::Runtime::SyntaxNode
    def content
      [elements[1].content, elements[0].content, elements[2].content]
    end
  end
  
  class Assignment < OperatorExpression
  end
  
  class AttributeAssignment < Treetop::Runtime::SyntaxNode
    def content
      # [:set, receiver, attribute, value]
      [:set, elements[0].content[1], elements[0].content[2], elements[2].content]
    end
  end
  
  class DotExpression < Treetop::Runtime::SyntaxNode
    def content
      if elements.length > 3 # Dot chain
        [elements[1].content, elements[2].content(elements[0].content), elements[3].content]
      else
        [elements[1].content, elements[0].content, elements[2].content]        
      end
    end
  end
  
  class DotChain < Treetop::Runtime::SyntaxNode
    def content(root)
      if elements.length == 2 # Last in the chain
        [elements[1].content, root, elements[0].content]
      else # x.y => [<identifier>x, <dot>, <dotchain> [<identifier>y, <dot>]]
        # The first dot in the list is the deepest, pair it with the root, and make it the root of the next in the line
        elements[2].content([elements[1].content, root, elements[0].content])
      end
    end
  end
  
  class ComparativeExpression < OperatorExpression
  end
  
  class AdditiveExpression < OperatorExpression
  end
  
  class MultitiveExpression < OperatorExpression
  end
  
  class Expression < Treetop::Runtime::SyntaxNode
    def content
      elements[0].content
    end
  end
  
  class Splat < Treetop::Runtime::SyntaxNode
    def content
      [:destructure, elements[0].content]
    end
  end

  class AssignmentOperator < Treetop::Runtime::SyntaxNode
    def content
      :"="
    end
  end

  class DotOperator < Treetop::Runtime::SyntaxNode
    def content
      :"."
    end
  end
  
  class AdditionOperator < Treetop::Runtime::SyntaxNode
    def content
      :+
    end
  end
  
  class SubtractionOperator < Treetop::Runtime::SyntaxNode
    def content
      :-
    end
  end
  
  class MultiplicationOperator < Treetop::Runtime::SyntaxNode
    def content
      :*
    end
  end
  
  class DivisionOperator < Treetop::Runtime::SyntaxNode
    def content
      :/
    end
  end
  
  class EqualityOperator < Treetop::Runtime::SyntaxNode
    def content
      :==
    end
  end
  
  class InequalityOperator < Treetop::Runtime::SyntaxNode
    def content
      :"!="
    end
  end
  
  class GreaterThanOperator < Treetop::Runtime::SyntaxNode
    def content
      :>
    end
  end
  
  class LessThanOperator < Treetop::Runtime::SyntaxNode
    def content
      :<
    end
  end

  class GreaterThanEqOperator < Treetop::Runtime::SyntaxNode
    def content
      :>=
    end
  end
  
  class LessThanEqOperator < Treetop::Runtime::SyntaxNode
    def content
      :<=
    end
  end
  
  class IdenticalityOperator < Treetop::Runtime::SyntaxNode
    def content
      :===
    end
  end
  
  class PrivateFunction < Treetop::Runtime::SyntaxNode
    def content
      [:private_function, *self.elements[0].content[1..-1]]
    end
  end

  class PrivateMethod < Treetop::Runtime::SyntaxNode
    def content
      [:private_method, *self.elements[0].content[1..-1]]
    end
  end
  
  class Function < Treetop::Runtime::SyntaxNode
    def content
      if elements.length > 2 # function with argument list
        [:function, *elements.map(&:content)]
      else # function with no argument list (i.e function x ... end)
        [:function, elements[0].content, [], elements[1].content]
      end
    end
  end
  
  class Method < Treetop::Runtime::SyntaxNode
    def content
      if elements.length > 2 # function with argument list
        [:method, *elements.map(&:content)]
      else # function with no argument list (i.e function x ... end)
        [:method, elements[0].content, [], elements[1].content]
      end
    end
  end

  class NoFunction < Treetop::Runtime::SyntaxNode
    def content
      [:nofunction, elements[0].content]
    end
  end

  class NoMethod < Treetop::Runtime::SyntaxNode
    def content
      [:nomethod, elements[0].content]
    end
  end

  class Argument < Treetop::Runtime::SyntaxNode
    def content
      {elements.first.content => elements.last.content}
    end
  end

  class ArgumentList < Treetop::Runtime::SyntaxNode
    def content
      if elements.length > 1
        elements[0].content + [elements[1].content]
      else
        elements.map(&:content)
      end
    end
  end
  
  class CommaSeparated < Treetop::Runtime::SyntaxNode
    def content
      if elements.length > 1
        [elements[0].content, *elements[1].content]
      else
        elements.map(&:content)
      end
    end
  end
  
  class If < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 2 # Only an if
        [:if, [[elements[0].content, elements[1].content]]]
      elsif elements.length == 3 # Elseif or else
        [:if, [[elements[0].content, elements[1].content], *elements[2].content]]
      elsif elements.length == 4 # Elseif and else
        [:if, [[elements[0].content, elements[1].content], *elements[2].content, *elements[3].content]]
      end
    end
  end
  
  class ElseIf < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 2 # Only one
        [[elements[0].content, elements[1].content]]
      else # Multiple elseifs
        [[elements[0].content, elements[1].content], *elements[2].content]
      end
    end
  end
  
  class Else < Treetop::Runtime::SyntaxNode
    def content
      [[:else, elements[0].content]]
    end
  end
  
  class Unless < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 2 # Just an unless
        [:unless, [[elements[0].content, elements[1].content]]]
      else # with an else
        [:unless, [[elements[0].content, elements[1].content], *elements[2].content]]
      end
    end
  end
  
  class While < Treetop::Runtime::SyntaxNode
    def content
      [:while, *elements.map(&:content)]
    end
  end
  
  class Module < Treetop::Runtime::SyntaxNode
    def content
      [:module, *elements.map(&:content)]
    end
  end
  
  class Class < Treetop::Runtime::SyntaxNode
    def content
      [:class, name, superclass, classbody]
    end
    
    def name
      elements[0].content
    end
    
    def superclass
      # The superclass will be the second entry if present, otherwise the second entry will be the last, an array of function definitions, etc.
      elements[1].content unless elements[1].content.is_a?(Array)
    end
    
    def classbody
      elements.last.content
    end
    
  end
  
  class FunctionInvocation < Treetop::Runtime::SyntaxNode
    def content
      if elements.length > 2 # Passed function specially or chained invocations
        if elements.last.content[0][0] == :lambda # Special function passing (get the content of the last element, it will be a list with a single lambda inside)
          [:"()", receiver, args + elements.last.content]
        elsif chained_call # chained invocation
          resolve([[:"()", receiver, args], *chained_call.content])
        else
          self
        end
      else
        [:"()", *elements.map(&:content)]
      end
    end
    
    def chained_call
      elements.last if (elements.last.is_a?(CallChain) or elements.last.is_a?(IndexInvokeChain))
    end
    
    def receiver
      elements.first.content
    end
    
    def args
      elements[1].content
    end
    
    def resolve(chained_calls, result = [])
      # Takes a chain of calls, deepest first, and constructs the call tree
      if chained_calls.empty?
        result
      else
        if result.empty?
          resolve(chained_calls[1..-1], chained_calls.first)
        else
          if chained_calls.first[0] == :"()" # method
            if chained_calls.first.length == 2 # basic reinvoking
              resolve(chained_calls[1..-1], [:"()", result, chained_calls.first[1]])
            else
              resolve(chained_calls[1..-1], [:"()", [:".", result, chained_calls.first[1]], chained_calls.first[2]])
            end
          elsif chained_calls.first[0] == :"[]" # index
            if chained_calls.first.length == 2 # basic reindexing
              resolve(chained_calls[1..-1], [:"[]", result, chained_calls.first[1]])
            else # advanced reindexing / dot chain
              if chained_calls.first[1].is_a?(Symbol)
                # dot chain
                resolve([[:[], chained_calls.first[2]]], [:".", result, chained_calls.first[1]])
              else
                resolve(chained_calls.first[2], [:"[]", result, chained_calls.first[1]])
              end
            end
          else # attribute
            resolve(chained_calls[1..-1], [:".", result, chained_calls.first])
          end
        end
      end
    end
    
  end

  class IndexAssignment < Treetop::Runtime::SyntaxNode
    def content
      assignee = elements[0].content
      return [:insert, assignee[1], assignee[2], elements[2].content]
    end
  end

  class Index < FunctionInvocation
    def content
      if elements.length > 2 # chained invocations
        if chained_call # chained invocation
          resolve([[:"[]", receiver, args], *chained_call.content])
        else
          self
        end
      else
        [:"[]", *elements.map(&:content)]
      end
    end    
  end

  class IndexArgumentList < ArgumentList
  end

  class IndexInvokeChain < Treetop::Runtime::SyntaxNode
    def content
      if elements.first.is_a?(IndexArgumentList)
        [[:[]] + elements.map(&:content)]
      else
        [[:"()"] + elements.map(&:content)]
      end
    end
  end

  class CallChain < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 1
        [elements.first.content]
      else
        [elements.first.content, *elements.last.content]
      end
    end
  end
  
  class Lambda < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 1 # No args
        [:lambda, [], elements[0].content]
      else
        [:lambda, *elements.map(&:content)]
      end
    end
  end
  
  # Used for special syntax for calling a function passing a lambda
  class PassedFunction < Treetop::Runtime::SyntaxNode
    def content
      [elements[0].content]
    end
  end
  
  class Return < Treetop::Runtime::SyntaxNode
    def content
      [:return, elements[0].content]
    end
  end
  
  class Begin < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 1
        [:begin, elements[0].content, {}, []]
      elsif elements.last.is_a? Ensure and elements.length == 2
        # Ensure, no rescue
        [:begin, elements[0].content, {}, elements[1].content]
      elsif elements.last.is_a? Ensure and elements.length == 3
        # Ensure, rescue
        [:begin, elements[0].content, Hash[elements[1].content], elements[2].content]
      else
        [:begin, elements[0].content, Hash[elements[1].content], []]
      end
    end
  end

  class Rescue < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 3
        # Binding to a variable
        [elements[0].content, [elements[1].content, elements[2].content]]
      else
        # Not binding to a variable
        [elements[0].content, [nil, elements[1].content]]
      end
    end
  end

  class RescueList < Treetop::Runtime::SyntaxNode
    def content
      if elements.length == 1
        [elements[0].content]
      else
        [elements[0].content, *elements[1].content]
      end
    end
  end

  class Ensure < Treetop::Runtime::SyntaxNode
    def content
      elements[0].content
    end
  end

end