module Iridium
  class Identifier < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_sym
    end
  end

  class IntegerLiteral < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_i
    end
  end

  class Assignment < Treetop::Runtime::SyntaxNode
    def content
      if elements[0].indirect?
        [elements[-1].indirection, *elements[0].content[1..-1], elements[1].content]
      else
        [:"=", elements[0].content, elements[1].content]
      end
    end
  end

  class Program < Treetop::Runtime::SyntaxNode
    def content
      elements.map(&:content)
    end
  end

  class StringLiteral < Treetop::Runtime::SyntaxNode
    def content
      text_value[1...-1]
    end
  end

  class FloatLiteral < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_f
    end
  end

  class AtomLiteral < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_sym
    end
  end

  class ArrayLiteral < Treetop::Runtime::SyntaxNode
    def content
      if elements[0].is_a?(Iridium::Expression)
        content = [elements[0].content]
      elsif elements[0]
        content = elements[0].content
      else
        content = []
      end
      [:array, content]
    end
  end

  class Operator < Treetop::Runtime::SyntaxNode
    def content
      text_value.to_sym
    end
  end

  class Expression < Treetop::Runtime::SyntaxNode
    def content
      [elements[1].content, elements[0].content, elements[2].content]
    end
    def build(base)
      content
    end
  end

  class MapLiteral < Treetop::Runtime::SyntaxNode
    def content
      if elements.first
        [:dictionary, elements.first.content]
      else
        [:dictionary, {}]
      end
    end
  end

  class MapContent < Treetop::Runtime::SyntaxNode
    def content
      key_content = elements[0].is_a?(Identifier) ? ":#{elements[0].text_value}".to_sym : elements[0].content
      base = {key_content => elements[1].content}
      if elements[2]
        base.merge! elements[2].content
      end
      base
    end
  end

  class ArrayContent < Treetop::Runtime::SyntaxNode
    def content
      next_content = if elements[1].is_a?(ArrayContent)
                       elements[1].content
                     else
                       [elements[1].content]
                     end
      [elements[0].content] + next_content
    end
  end

  class Invocation < Treetop::Runtime::SyntaxNode
    def build(base)
      if elements.empty?
        [:"()", base, []]
      elsif elements[0].is_a?(Arguments)
        [:"()", base, elements[0].content]
      else
        [:"()", base, [elements[0].content]]
      end
    end
  end

  class IndexExpression < Treetop::Runtime::SyntaxNode
    def build(base)
      if elements[0].is_a?(Arguments)
        [:"[]", base, elements[0].content]
      elsif elements[-1].respond_to? :content
        [:"[]", base, elements.map(&:content)]
      else
        elements[-1].build([:"[]", base, elements[0...-1].map(&:content)])
      end
    end
    def indirection
      :insert
    end
  end

  class AttributeExpression < Treetop::Runtime::SyntaxNode
    def build(base)
      if elements[-1].respond_to? :build
        elements[-1].build([:".", base, elements[0].content])
      else
        [:".", base, elements[0].content]
      end
    end
    def indirection
      :set
    end
  end

  class Term < Treetop::Runtime::SyntaxNode
    def indirect?
      elements[1].is_a?(IndexExpression) || elements[1].is_a?(AttributeExpression)
    end
    def indirection
      elements[1].indirection
    end
    def content
      if elements.size == 2
        # w/ trailer
        elements[1].build(elements[0].content)
      else
        elements[0].content
      end
    end
    def build(base)
      content
    end
  end

  class Arguments < Treetop::Runtime::SyntaxNode
    def content
      if elements[1].is_a?(Arguments)
        [elements[0].content] + elements[1].content
      else
        elements.map(&:content)
      end
    end
  end

end
