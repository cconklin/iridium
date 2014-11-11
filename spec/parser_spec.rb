require_relative "../lib/parser"

describe Parser do
  let (:parser) { Parser.new }
  describe "assignments" do
    it "should parse with integer literals" do
      expect(parser.parse("x = 5")).to eq([[:"=", :x, 5]])
    end

    it "should parse with addition" do
      expect(parser.parse("x = 2 + 3")).to eq([[:"=", :x, [:+, 2, 3]]])
    end
  
    it "should parse with multiplication" do
      expect(parser.parse("x = 2 * 3")).to eq([[:"=", :x, [:*, 2, 3]]])    
    end
  
    it "should parse with subtraction" do
      expect(parser.parse("x = 2 - 3")).to eq([[:"=", :x, [:-, 2, 3]]])        
    end
  
    it "should parse with division" do
      expect(parser.parse("x = 2 / 3")).to eq([[:"=", :x, [:/, 2, 3]]])   
    end
  
    it "should parse with computation" do
      expect(parser.parse("x = 2 / 3 + 6 * (7 - 4)")).to eq([[:"=", :x, [:+, [:/, 2, 3], [:*, 6, [:-, 7, 4]]]]])      
    end
  
    it "should parse with function calls with no arguments" do
      expect(parser.parse("x = foo()")).to eq([[:"=", :x, [:"()", :foo, []]]])
    end
  
    it "should parse with function calls with one argument" do
      expect(parser.parse("x = foo(bar)")).to eq([[:"=", :x, [:"()", :foo, [:bar]]]])
    end
  
    it "should parse with function calls with many arguments" do
      expect(parser.parse("x = foo(bar, baz, 3, boo())")).to eq([[:"=", :x, [:"()", :foo, [:bar, :baz, 3, [:"()", :boo, []]]]]])
    end
  
    it "should parse with string literals" do
      expect(parser.parse("x = \"foo\"")).to eq([[:"=", :x, "foo"]])
    end
    it "should parse from other variables" do
      expect(parser.parse("x = bar")).to eq([[:"=", :x, :bar]])
    end
  
    it "should parse with float literals" do
      expect(parser.parse("x = 2.7")).to eq([[:"=", :x, 2.7]])    
    end
    
    it "should parse with annonymous functions" do
      expect(parser.parse("x = -> return 4 end")).to eq([[:"=", :x, [:lambda, [], [[:return, 4]]]]])
    end
    
  end
  describe "function definitions" do
    
    it "should parse with no arguments" do
      func = <<-END
      function x()
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [], [[:"=", :y, 5]]]])
    end
    
    it "should parse with no arguments and no parenthesis" do
      func = <<-END
      function x
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [], [[:"=", :y, 5]]]])
    end
    
    it "should not parse with arguments and no parenthesis" do
      func = <<-END
      function x z
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)
    end
    
    it "should parse with arguments with whitespace" do
      func = <<-END
      function x ( z )
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [:z], [[:"=", :y, 5]]]])      
    end
    
    it "should parse with arguments without whitespace" do
      func = <<-END
      function x(z)
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [:z], [[:"=", :y, 5]]]])      
    end
    
    it "should parse with many arguments" do
      func = <<-END
      function x(z, y, w)
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [:z, :y, :w], [[:"=", :y, 5]]]])
    end
    
    it "should not parse with integer literals in the argument list" do
      func = <<-END
      function x(5)
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)      
    end

    it "should not parse with float literals in the argument list" do
      func = <<-END
      function x(5.1)
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)      
    end
    
    it "should not parse with string literals in the argument list" do
      func = <<-END
      function x("foo")
        y = 5
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)      
    end
    
    it "should parse with multiple statements" do
      func = <<-END
      function x
        z = 10
        y = 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [], [[:"=", :z, 10], [:"=", :y, 5]]]])
    end
    
    it "should allow return values" do
      func = <<-END
      function x
        return 5
      end
      END
      expect(parser.parse(func)).to eq([[:function, :x, [], [[:return, 5]]]])
    end
    
    it "should not allow functions to be defined in functions" do
      func = <<-END
      function x
        function y
          z = 5
        end
      end
      END
      expect { parser.parse(func) }.to raise_error(ParseError)      
    end
    
  end
  describe "annonymous functions" do
    it "should recognize the -> syntax" do
      expect(parser.parse("x = -> y = 2 end")).to eq([[:"=", :x, [:lambda, [], [[:"=", :y, 2]]]]])
    end
    it "should let them accept arguments" do
      expect(parser.parse("x = -> (y) y = 2 end")).to eq([[:"=", :x, [:lambda, [:y], [[:"=", :y, 2]]]]])      
    end
    it "should not require that arguments have whitespace after the ->" do
      expect(parser.parse("x = ->(y) y = 2 end")).to eq([[:"=", :x, [:lambda, [:y], [[:"=", :y, 2]]]]])      
    end
    it "should let them accept multiple arguments" do
      expect(parser.parse("x = -> (y, z) y = 2 end")).to eq([[:"=", :x, [:lambda, [:y, :z], [[:"=", :y, 2]]]]])      
    end
    it "should let them have multiple expressions" do
      expect(parser.parse("x = -> y = 2\ny = 3 end")).to eq([[:"=", :x, [:lambda, [], [[:"=", :y, 2], [:"=", :y, 3]]]]])            
    end
    it "should not parse when arguments are not in parenthesis" do
      expect{ parser.parse("x = -> y y = 2 end") }.to raise_error(ParseError)      
    end
    it "should allow them to return values" do
      expect(parser.parse("x = -> return 2 end")).to eq([[:"=", :x, [:lambda, [], [[:return, 2]]]]])
    end
    it "should allow multiple lines" do
      func = <<-END
        x = ->
          return 6
        end
      END
      expect(parser.parse(func)).to eq([[:"=", :x, [:lambda, [], [[:return, 6]]]]])
    end
    it "should be able to return annonymous functions" do
      func = <<-END
        x = ->
          return ->
            return 5
          end
        end
      END
      expect(parser.parse(func)).to eq([[:"=", :x, [:lambda, [], [[:return, [:lambda, [], [[:return, 5]]]]]]]])      
    end
  end
  describe "loops such as" do
    describe "while" do
      it "should parse with an identifier" do
        expect(parser.parse("while true x = 5 end")).to eq([[:while, :true, [[:"=", :x, 5]]]])
      end
      it "should parse with a condition" do
        expect(parser.parse("while 2 > 3 x = 5 end")).to eq([[:while, [:>, 2, 3], [[:"=", :x, 5]]]])
      end
      it "should parse with multiple statements" do
        expect(parser.parse("while 2 > 3 x = 5 some_func() end")).to eq([[:while, [:>, 2, 3], [[:"=", :x, 5], [:"()", :some_func, []]]]])        
      end
    end
    describe "for" do
      it "should parse with identifiers" do
        expect(parser.parse("for foo in bar print(foo) end")).to eq([[:for, :foo, :bar, [[:"()", :print, [:foo]]]]])
      end
      it "should parse with a literal as the loopee" do
        expect(parser.parse("for foo in 5 print(foo) end")).to eq([[:for, :foo, 5, [[:"()", :print, [:foo]]]]])        
      end
      it "should parse with a function call as the loopee" do
        expect(parser.parse("for foo in some_result() print(foo) end")).to eq([[:for, :foo, [:"()", :some_result, []], [[:"()", :print, [:foo]]]]])        
      end
      it "should not parse with a literal as the looping variable" do
        expect { parser.parse "for 5 in foo print(foo) end" }.to raise_error(ParseError)
      end
    end
  end
  describe "conditionals such as" do
    describe "if" do
      it "should parse with an identifier" do
        expect(parser.parse("if true x = 5 end")).to eq([[:if, :true, [[:"=", :x, 5]]]])
      end
      it "should parse with a literal" do
        expect(parser.parse("if 5 x = 5 end")).to eq([[:if, 5, [[:"=", :x, 5]]]])        
      end
      it "should parse with a comparison" do
        expect(parser.parse("if 5 > 3 x = 5 end")).to eq([[:if, [:>, 5, 3], [[:"=", :x, 5]]]])                
      end
    end
  end
  describe "function invocations" do
    it "should parse with no arguments" do
      expect(parser.parse("foo()")).to eq([[:"()", :foo, []]])
    end
    it "should parse with arguments" do
      expect(parser.parse("foo(bar, 3, \"foo\")")).to eq([[:"()", :foo, [:bar, 3, "foo"]]])      
    end
    it "should parse with annonymous functions and no arguments" do
      expect(parser.parse("foo() -> return 5 end")).to eq([[:"()", :foo, [[:lambda, [], [[:return, 5]]]]]])      
    end
    it "should parse with annonymous functions and arguments" do
      expect(parser.parse("foo (5, 6) -> return 5 end")).to eq([[:"()", :foo, [5, 6, [:lambda, [], [[:return, 5]]]]]])      
    end
    it "should parse with annonymous function and no arguments with no parentheses" do
      expect(parser.parse("foo -> return 5 end")).to eq([[:"()", :foo, [[:lambda, [], [[:return, 5]]]]]])            
    end
    it "should not parse with annonymous function and no parentheses" do
      expect { parser.parse("foo 5 -> return 5 end") }.to raise_error(ParseError)          
    end
    
  end
end