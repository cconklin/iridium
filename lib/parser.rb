require 'treetop'

base_path = File.join(File.expand_path(File.dirname(__FILE__)), "parser")
Treetop.load(File.join(base_path, "iridium_parser.treetop"))

require File.join(base_path, "iridium.rb")

class ParseError < Exception
end

class Parser
  
  def initialize
    @parser = IridiumParser.new
  end
  
  def parse(data)
    tree = @parser.parse(data)
    if tree.nil?
      @parser.failure_reason =~ /^(Expected .+) after/m
      raise ParseError, "#{$1.gsub("\n", '$NEWLINE')}:\n#{data.lines.to_a[@parser.failure_line - 1]}\n#{'~' * (@parser.failure_column - 1)}^"
    else
      clean_tree(tree)
      return tree.content
    end
  end
  
  def clean_tree(root_node)
    return if(root_node.elements.nil?)
    root_node.elements.delete_if{|node| node.class.name == "Treetop::Runtime::SyntaxNode" }
    root_node.elements.each {|node| self.clean_tree(node) }
  end
  
  
end