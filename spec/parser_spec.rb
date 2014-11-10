require_relative "../lib/parser"

describe Parser do
  let (:parser) { Parser.new }
  describe "parsing assignments" do
    it "should parse assignments that are simple" do
      expect(parser.parse("x = 5")).to eq([[:"=", :x, 5]])
    end

    it "should parse assignments with addition" do
      expect(parser.parse("x = 2 + 3")).to eq([[:"=", :x, [:+, 2, 3]]])
    end
  
    it "should parse assignments with multiplication" do
      expect(parser.parse("x = 2 * 3")).to eq([[:"=", :x, [:*, 2, 3]]])    
    end
  
    it "should parse assignments with subtraction" do
      expect(parser.parse("x = 2 - 3")).to eq([[:"=", :x, [:-, 2, 3]]])        
    end
  
    it "should parse assignments with division" do
      expect(parser.parse("x = 2 / 3")).to eq([[:"=", :x, [:/, 2, 3]]])   
    end
  
    it "should parse assignments with computation" do
      expect(parser.parse("x = 2 / 3 + 6 * (7 - 4)")).to eq([[:"=", :x, [:+, [:/, 2, 3], [:*, 6, [:-, 7, 4]]]]])      
    end
  
    it "should parse assignments with function calls with no arguments" do
      expect(parser.parse("x = foo()")).to eq([[:"=", :x, [:"()", :foo, []]]])
    end
  
    it "should parse assignments with function calls with one argument" do
      expect(parser.parse("x = foo(bar)")).to eq([[:"=", :x, [:"()", :foo, [:bar]]]])
    end
  
    it "should parse assignments with function calls with many arguments" do
      expect(parser.parse("x = foo(bar, baz, 3, boo())")).to eq([[:"=", :x, [:"()", :foo, [:bar, :baz, 3, [:"()", :boo, []]]]]])
    end
  
    it "should parse assignments with string literals" do
      expect(parser.parse("x = \"foo\"")).to eq([[:"=", :x, "foo"]])
    end
    it "should parse assignments from other variables" do
      expect(parser.parse("x = bar")).to eq([[:"=", :x, :bar]])
    end
  
    it "should parse assignments with float literals" do
      expect(parser.parse("x = 2.7")).to eq([[:"=", :x, 2.7]])    
    end
  end
  describe "parsing function definitions" do
    
    it "should parse a function with no arguments" do
      func = <<-END
      function x()
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [], [[:"=", :y, 5]]]])
    end
    
    it "should parse a function with no arguments and no parenthesis" do
      func = <<-END
      function x
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [], [[:"=", :y, 5]]]])
    end
    
    it "should not parse a function with arguments and no parenthesis" do
      func = <<-END
      function x z
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)
    end
    
    it "should parse a function with arguments with whitespace" do
      func = <<-END
      function x ( z )
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [:z], [[:"=", :y, 5]]]])      
    end
    
    it "should parse a function with arguments without whitespace" do
      func = <<-END
      function x(z)
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [:z], [[:"=", :y, 5]]]])      
    end
    
    it "should parse a function with many arguments" do
      func = <<-END
      function x(z, y, w)
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [:z, :y, :w], [[:"=", :y, 5]]]])
    end
    
    it "should not parse a function with integer literals in the argument list" do
      func = <<-END
      function x(5)
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)      
    end

    it "should not parse a function with float literals in the argument list" do
      func = <<-END
      function x(5.1)
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)      
    end
    
    it "should not parse a function with string literals in the argument list" do
      func = <<-END
      function x("foo")
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)      
    end
    
    it "should parse functions with multiple statements" do
      func = <<-END
      function x
        z = 10
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [], [[:"=", :z, 10], [:"=", :y, 5]]]])
    end
    
  end
  
end