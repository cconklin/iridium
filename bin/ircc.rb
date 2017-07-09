#!/usr/bin/env ruby

require 'optparse'
require_relative "../ircc/parser"
require_relative "../ircc/translator"
require_relative "../ircc/generator"
require_relative "../ircc/compiler"
require_relative "../ircc/loader"

options = {output: "a.out", debug: false, link: true}

parser = OptionParser.new do |opts|
  opts.banner = "Usage: ircc [options] file"
  opts.on "-o", "--output FILE", "Output Binary" do |o|
    options[:output] = o 
  end
  opts.on "-g", "Emit Debug Symbols" do |g|
    options[:debug] = g
  end
  opts.on "-c", "Only run compile and assemble steps" do
    options[:link] = false
  end
  opts.on "--main=MAIN", "top-level name" do |mn|
    options[:main_name] = mn
  end
end

parser.parse!

if ARGV.empty?
  puts parser
  exit 1
end

loader = Loader.new
tree = loader.load_from_file ARGV[0], Dir.pwd

processed_tree = Translator.translate! tree
generator = Generator.new processed_tree[:callables],
                          processed_tree[:tree],
                          options[:link] ? "ir_user_main" : options[:main_name],
                          options[:link]

Compiler.compile generator.generate,
                 output: options[:output],
                 debug: options[:debug],
                 link: options[:link]

