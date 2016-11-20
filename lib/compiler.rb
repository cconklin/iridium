module Compiler
  extend self

  def compile(code, output: "a.out", debug: false)
    IO.popen "/usr/bin/clang -o #{output} #{debug ? '-g' : ''} -xc -", "w" do |io|
      io.write code
    end
  end
end

