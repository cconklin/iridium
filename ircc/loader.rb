require_relative 'parser'

class Loader

  CORE_FILES = []
  LOAD_PATH = [
    # Iridium files to bootstrap the language
    File.join(File.dirname(File.dirname(__FILE__)), "iridium", "iridium")
  ]

  def initialize
    @loaded_files = []
    @file_stack = []
  end

  def self.locate_file(filename, base)
    # Try relative first
    relative = File.absolute_path filename, base
    if File.exists? relative
      return relative
    else
      LOAD_PATH.each do |path|
        absolute = File.absolute_path filename, path
        if File.exists? absolute
          return absolute
        end
      end
    end
    raise LoadError.new(filename)
  end

  def load_from_file(filename, base)
    filepath = Loader.locate_file filename, base
    if @loaded_files.include? filepath
      # Already been loaded -- give an empty tree
      [nil]
    else
      # Not loaded yet -- load the tree
      @loaded_files << filepath
      content = File.read filepath
      parser = Parser.new
      tree = parser.parse(content)
      # Set the __FILE__ constant
      @file_stack << filename
      tree.unshift [:"=", :__FILE__, filename]
      tree.unshift *(CORE_FILES - @loaded_files).map {|f| [:require, f]}
      tree = load_requires!(tree, File.dirname(filepath)).compact
      @file_stack.pop
      tree
    end
  end

  def load_requires!(tree, base)
    tree.each_with_index do |node, index|
      if node.respond_to? :each
        if node.first == :require
          sub_require!(tree, index, base)
        else
          load_requires!(node, base)
        end
      end
    end
  end

  def sub_require!(tree, index, base)
    # node is the syntax node of the require
    node = tree[index]
    # load the required file
    replacement_tree = load_from_file(node[1] + ".ir", base)
    # Restore the __FILE__ variable
    replacement_tree << [:"=", :__FILE__, @file_stack.last]
    # remove the require nodes
    tree.delete_at index
    # replace the require node with what was required
    tree[index, 0] = replacement_tree
  end

end
