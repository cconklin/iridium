#!/usr/bin/env ruby

require 'optparse'
require_relative "../lib/parser"
require_relative "../lib/translator"
require_relative "../lib/generator"
require_relative "../lib/compiler"

options = {output: "a.out", debug: false}

parser = OptionParser.new do |opts|
  opts.banner = "Usage: ircc [options] file"
  opts.on "-o", "--output FILE", "Output Binary" do |o|
    options[:output] = o 
  end
  opts.on "-g", "Emit Debug Symbols" do |g|
    options[:debug] = g
  end
end

parser.parse!

if ARGV.empty?
  puts parser
  exit 1
end

content = File.read ARGV[0]
parser = Parser.new
tree = parser.parse content

processed_tree = Translator.translate! tree

generator = Generator.new(processed_tree[:callables], processed_tree[:tree])

Compiler.compile(generator.generate, output: options[:output], debug: options[:debug])

