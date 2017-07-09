require 'securerandom'

module Compiler
  extend self

  IRIDIUM = File.expand_path File.join(File.dirname(__FILE__), "..", "iridium", "objects", "ir.o")
  LDFLAGS = `pkg-config --libs bdw-gc`.strip

  def compile(code, output: "a.out", debug: false, link: true)
    fname = SecureRandom.hex + ".c"
    command = if link
      "/usr/bin/clang #{LDFLAGS} -o #{output} #{debug ? '-g' : ''} #{IRIDIUM} -xc #{debug ? fname : '-'}"
    else
      "/usr/bin/clang -o #{output} -c #{debug ? '-g' : '' } -xc #{debug ? fname : '-'}"
    end
    if debug
      puts command
      File.open(fname, "w") do |f|
        f.write code
      end
    end
    IO.popen command, "w" do |io|
      io.write code unless debug
    end
  end

end

