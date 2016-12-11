require_relative 'parser'

class Loader

  CORE_FILES = [File.join(File.dirname(File.dirname(__FILE__)), "src", "iridium", "core")]
  LOAD_PATH = [
    # Iridium files to bootstrap the language
    File.join(File.dirname(File.dirname(__FILE__)), "src", "iridium")
  ]

  def initialize
    @loaded_files = []
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
      tree.unshift *(CORE_FILES - @loaded_files).map {|f| [:require, f]}
      load_requires!(tree, File.dirname(filepath)).compact
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
    # remove the require nodea
    tree.delete_at index
    # replace the require node with what was required
    tree[index, 0] = replacement_tree
  end

end
