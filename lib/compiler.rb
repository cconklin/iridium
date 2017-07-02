module Compiler
  extend self

  IRIDIUM_A = File.expand_path File.join(File.dirname(__FILE__), "..", "src", "iridium.a")

  def compile(code, output: "a.out", debug: false, link: true)
    command = if link
      "/usr/bin/clang -o #{output} #{debug ? '-g' : ''} #{IRIDIUM_A} -O3 -xc -"
    else
      "/usr/bin/clang -o #{output} -c -xc -"
    end
    IO.popen command, "w" do |io|
      io.write code
    end
  end

end

