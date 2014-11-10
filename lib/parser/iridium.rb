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
  
  class AssignmentOperator < Treetop::Runtime::SyntaxNode
    def content
      :"="
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
  end
  
  class InequalityOperator < Treetop::Runtime::SyntaxNode
  end
  
  class GreaterThanOperator < Treetop::Runtime::SyntaxNode
  end
  
  class LessThanOperator < Treetop::Runtime::SyntaxNode
  end
  
  class Function < Treetop::Runtime::SyntaxNode
    def content
      if elements.length > 2 # function with argument list
        [:function, elements[0].content, elements[1].content, elements[2].content]
      else # function with no argument list (i.e function x ... end)
        [:function, elements[0].content, [], elements[1].content]
      end
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
  end
  
  class While < Treetop::Runtime::SyntaxNode
  end
  
  class For < Treetop::Runtime::SyntaxNode
  end
  
  class Module < Treetop::Runtime::SyntaxNode
    def content
      [:module, elements[0].content, elements[1].content]
    end
  end
  
  class Class < Treetop::Runtime::SyntaxNode
    def content
      [:class, elements[0].content, elements[1].content]
    end
  end
  
  class FunctionInvocation < Treetop::Runtime::SyntaxNode
    def content
      [:"()", elements[0].content, elements[1].content]
    end
  end
  
end