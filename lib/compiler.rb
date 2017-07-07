module Compiler
  extend self

  IRIDIUM = File.expand_path File.join(File.dirname(__FILE__), "..", "src", "ir.o")
  LDFLAGS = `pkg-config --libs bdw-gc`.strip

  def compile(code, output: "a.out", debug: false, link: true)
    command = if link
      "/usr/bin/clang #{LDFLAGS} -o #{output} #{debug ? '-g' : ''} #{IRIDIUM} -xc -"
    else
      "/usr/bin/clang -o #{output} -c #{debug ? '-g' : '' } -xc -"
    end
    IO.popen command, "w" do |io|
      io.write code
    end
  end

end

