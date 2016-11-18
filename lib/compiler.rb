module Compiler
  extend self

  def compile(code, output: "a.out")
    IO.popen "/usr/bin/clang -o #{output} -xc -", "w" do |io|
      io.write code
    end
  end
end

