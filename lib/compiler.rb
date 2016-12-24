module Compiler
  extend self

  IRIDIUM_A = File.expand_path File.join(File.dirname(__FILE__), "..", "src", "iridium.a")

  def compile(code, output: "a.out", debug: false)
    IO.popen "/usr/bin/clang -o #{output} #{debug ? '-g' : ''} #{IRIDIUM_A} -xc -", "w" do |io|
      io.write code
    end
  end
end

