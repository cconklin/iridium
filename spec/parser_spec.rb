require_relative "../lib/parser"

describe Parser do
  let (:parser) { Parser.new }
  describe "assignments" do
    it "should parse with integer literals" do
      expect(parser.parse("x = 5")).to eq([[:"=", :x, 5]])
    end
    
    it "should parse with atom literals" do
      expect(parser.parse("x = :foo")).to eq([[:"=", :x, :":foo"]])
    end
    
    it "should parse with empty list literals" do
      expect(parser.parse("x = []")).to eq([[:"=", :x, [:list, []]]])
    end
    
    it "should parse with list literals with one element" do
      expect(parser.parse("x = [2 + 4]")).to eq([[:"=", :x, [:list, [[:+, 2, 4]]]]])      
    end
    
    it "should parse with list literals" do
      expect(parser.parse("x = [1, :foo, 2 + 3, \"bar\"]")).to eq([[:"=", :x, [:list, [1, :":foo", [:+, 2, 3], "bar"]]]])
    end
    
    it "should parse with empty tuple literals" do
      expect(parser.parse("x = {}")).to eq([[:"=", :x, [:tuple, []]]])      
    end
    
    it "should parse with tuple literals" do
      expect(parser.parse("x = {1, :foo, 2 + 3, \"bar\"}")).to eq([[:"=", :x, [:tuple, [1, :":foo", [:+, 2, 3], "bar"]]]])      
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
    
    describe "returning values" do
      it "should allow for complex return statements" do
        expect(parser.parse("return self.foo.bar().baz()")).to eq([[:return, [:"()", [:".", [:"()", [:".", [:".", :self, :foo], :bar], []], :baz], []]]])
      end
    end
    
    describe "private functions" do
      it "should allow private declarations of functions" do
        func = <<-END
        private function x
          y = 5
        end
        END
        expect(parser.parse(func)).to eq([[:private_function, :x, [], [[:"=", :y, 5]]]])
      end
    end
    
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
  end
  describe "conditionals such as" do
    describe "if" do
      it "should parse with an identifier" do
        expect(parser.parse("if true x = 5 end")).to eq([[:if, [[:true, [[:"=", :x, 5]]]]]])
      end
      it "should parse with a literal" do
        expect(parser.parse("if 5 x = 5 end")).to eq([[:if, [[5, [[:"=", :x, 5]]]]]])        
      end
      it "should parse with a comparison" do
        expect(parser.parse("if 5 > 3 x = 5 end")).to eq([[:if, [[[:>, 5, 3], [[:"=", :x, 5]]]]]])                
      end
      it "should allow an elseif statement" do
        expect(parser.parse("if 5 > 3 x = 5 elseif x < 6 x = 4 end")).to eq([[:if, [[[:>, 5, 3], [[:"=", :x, 5]]], [[:<, :x, 6], [[:"=", :x, 4]]]]]])                        
      end
      it "should allow two elseif statements" do
        expect(parser.parse("if 5 > 3 x = 5 elseif x < 6 x = 4 elseif x == 7 x = 6 end")).to eq([[:if, [[[:>, 5, 3], [[:"=", :x, 5]]], [[:<, :x, 6], [[:"=", :x, 4]]], [[:==, :x, 7], [[:"=", :x, 6]]]]]])        
      end
      it "should allow many elseif statements" do
        expect(parser.parse("if 5 > 3 x = 5 elseif x < 6 x = 4 elseif x == 7 x = 6 elseif x == 5 x = 4 end")).to eq([[:if, [[[:>, 5, 3], [[:"=", :x, 5]]], [[:<, :x, 6], [[:"=", :x, 4]]], [[:==, :x, 7], [[:"=", :x, 6]]], [[:==, :x, 5], [[:"=", :x, 4]]]]]])        
      end
      it "should not allow elseif statements without an if" do
        expect { parser.parse("elsif x == 5 x = 4 end") }.to raise_error ParseError
      end
      it "should allow else statements" do
        expect(parser.parse("if 5 > 3 x = 5 else x = 4 end")).to eq([[:if, [[[:>, 5, 3], [[:"=", :x, 5]]], [:else, [[:"=", :x, 4]]]]]])                        
      end
      it "should only allow one else statement" do
        expect { parser.parse("if 5 > 3 x = 5 else x = 4 else x = 5 end") }.to raise_error ParseError                               
      end
      it "should allow elseif and else statements" do
        expect(parser.parse("if 5 > 3 x = 5 elseif x < 6 x = 4 elseif x == 7 x = 6 else x = 4 end")).to eq([[:if, [[[:>, 5, 3], [[:"=", :x, 5]]], [[:<, :x, 6], [[:"=", :x, 4]]], [[:==, :x, 7], [[:"=", :x, 6]]], [:else, [[:"=", :x, 4]]]]]])        
      end
      it "should not allow else statements on their own" do
        expect { parser.parse("else x = 4 end") }.to raise_error ParseError        
      end
    end
    describe "unless" do
      it "should parse with an identifier" do
        expect(parser.parse("unless true x = 5 end")).to eq([[:unless, [[:true, [[:"=", :x, 5]]]]]])
      end
      it "should parse with a literal" do
        expect(parser.parse("unless 5 x = 5 end")).to eq([[:unless, [[5, [[:"=", :x, 5]]]]]])        
      end
      it "should parse with a comparison" do
        expect(parser.parse("unless 5 > 3 x = 5 end")).to eq([[:unless, [[[:>, 5, 3], [[:"=", :x, 5]]]]]])                
      end
      it "should allow else statements" do
        expect(parser.parse("unless 5 > 3 x = 5 else x = 4 end")).to eq([[:unless, [[[:>, 5, 3], [[:"=", :x, 5]]], [:else, [[:"=", :x, 4]]]]]])                        
      end
      it "should only allow one else statement" do
        expect { parser.parse("unless 5 > 3 x = 5 else x = 4 else x = 5 end") }.to raise_error ParseError                               
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
    
    it "should parse with arguments and attributes" do
      expect(parser.parse("foo(bar.baz, 3, \"foo\")")).to eq([[:"()", :foo, [[:".", :bar, :baz], 3, "foo"]]])            
    end
    
    it "should parse with arguments and expressions" do
      expect(parser.parse("foo(3 + bar.baz)")).to eq([[:"()", :foo, [[:+, 3, [:".", :bar, :baz]]]]])            
    end
    
    it "should parse with arguments and expressions and attributes" do
      expect(parser.parse("foo(a.b + bar.baz)")).to eq([[:"()", :foo, [[:+, [:".", :a, :b], [:".", :bar, :baz]]]]])            
    end
    
    it "should parse with arguments and multiplication and attributes" do
      expect(parser.parse("foo(a.b * bar.baz)")).to eq([[:"()", :foo, [[:*, [:".", :a, :b], [:".", :bar, :baz]]]]])            
    end
    
    it "should allow methods chains terminating in passed functions" do
      expect(parser.parse("self.foo().bar.x().y() -> return 5 end")).to eq([[:"()", [:".", [:"()", [:".", [:".", [:"()", [:".", :self, :foo], []], :bar], :x], []], :y], [[:lambda, [], [[:return, 5]]]]]])
    end
    
    it "should allow method chains with passed functions in the middle" do
      expect(parser.parse("self.foo(-> return 5 end).bar.x()")).to eq([[:"()", [:".", [:".", [:"()", [:".", :self, :foo], [[:lambda, [], [[:return, 5]]]]], :bar], :x], []]])
    end
    
    it "should not allow special function passing in the middle of call chains" do
      expect { parser.parse "self.foo -> return 5 end.bar" }.to raise_error ParseError
    end
    
  end
  
  describe "getting attributes" do
    it "should get non function attributes" do
      expect(parser.parse("x = foo.x")).to eq([[:"=", :x, [:".", :foo, :x]]])
    end
    
    it "should get attributes of attributes" do
      expect(parser.parse("x = foo.x.y")).to eq([[:"=", :x, [:".", [:".", :foo, :x], :y]]])
    end
    
    it "should get attributes of attributes of attributes" do
      expect(parser.parse("x = foo.x.y.z.w")).to eq([[:"=", :x, [:".", [:".", [:".", [:".", :foo, :x], :y], :z], :w]]])
    end
    
    it "should allow getting attributes of the results of function calls" do
      expect(parser.parse("x = foo().x")).to eq([[:"=", :x, [:".", [:"()", :foo, []], :x]]])      
    end
    
    it "should allow invoking methods in assignment" do
      expect(parser.parse("x = (foo.x)()")).to eq([[:"=", :x, [:"()", [:".", :foo, :x], []]]])
    end
    
    it "should allow invoking methods" do
      expect(parser.parse("(foo.x)()")).to eq([[:"()", [:".", :foo, :x], []]])
    end
    
    it "should give the dot operator higher precedence than invocation" do
      expect(parser.parse("foo.x()")).to eq([[:"()", [:".", :foo, :x], []]])
    end
    
    it "should allow deep method calls" do
      expect(parser.parse("foo.x.y()")).to eq([[:"()", [:".", [:".", :foo, :x], :y], []]])    
    end
    
    it "should allow method calls anywhere in the chain" do
      expect(parser.parse("foo().x()")).to eq([[:"()", [:".", [:"()", :foo, []], :x], []]])
    end
    
    it "should allow method calls with arguments anywhere in the chain" do
      expect(parser.parse("foo(z).x(y, 2)")).to eq([[:"()", [:".", [:"()", :foo, [:z]], :x], [:y, 2]]])
    end
    
    it "should prioritize attributes over addition" do
      expect(parser.parse("x = 2 + foo.x()")).to eq([[:"=", :x, [:+, 2, [:"()", [:".", :foo, :x], []]]]])      
    end
    
    it "should prioritize attributes over multiplication" do
      expect(parser.parse("x = 2 * foo.x()")).to eq([[:"=", :x, [:*, 2, [:"()", [:".", :foo, :x], []]]]])      
    end
    
    it "should allow the setting of attributes" do
      expect(parser.parse("x.y = 2")).to eq([[:set, :x, :y, 2]])
    end
    
    it "should allow the setting of deep attributes" do
      expect(parser.parse("x.y.z.q = 2")).to eq([[:set, [:".", [:".", :x, :y], :z], :q, 2]])
    end
    
    it "should allow methods to appear anywhere in the chain" do
      expect(parser.parse("self.foo().bar.x().baz")).to eq([[:".", [:"()", [:".", [:".", [:"()", [:".", :self, :foo], []], :bar], :x], []], :baz]])
    end
    
  end
  
  describe "class definitions" do
    it "should parse empty class definitions" do
      expect(parser.parse("class Foo end")).to eq([[:class, :Foo, nil, []]])
    end
    
    it "should be able to inherit from other classes" do      
      expect(parser.parse("class Foo < Bar end")).to eq([[:class, :Foo, :Bar, []]])
    end
    
    it "should be able to contain function definitions" do
      expect(parser.parse("class Foo function x end end")).to eq([[:class, :Foo, nil, [[:function, :x, [], []]]]])      
    end
    
    it "should be able to contain attribute assignments" do
      expect(parser.parse("class Foo self.x = 5 end")).to eq([[:class, :Foo, nil, [[:set, :self, :x, 5]]]])      
    end
    
    it "should not be able to contain local assignments" do
      expect { parser.parse "class Foo x = 5 end" }.to raise_error ParseError
    end
    
    it "should not be able to contain class definitions" do
      expect { parser.parse "class Foo class Bar end end" }.to raise_error ParseError      
    end
    
    it "should not be able to contain module definitions" do
      expect { parser.parse "class Foo module Bar end end" }.to raise_error ParseError      
    end
    
    it "should be able to include modules" do
      expect( parser.parse("class Foo self.include(Bar) end") ).to eq([[:class, :Foo, nil, [[:"()", [:".", :self, :include], [:Bar]]]]])
    end
    
  end
  
  describe "module definitions" do
    it "should allow defining modules at the top level" do
      expect(parser.parse("module F end")).to eq([[:module, :F, []]])
    end
    
    it "should be able to contain function definitions" do
      expect(parser.parse("module Foo function x end end")).to eq([[:module, :Foo, [[:function, :x, [], []]]]])      
    end
    
    it "should be able to contain attribute assignments" do
      expect(parser.parse("module Foo self.x = 5 end")).to eq([[:module, :Foo, [[:set, :self, :x, 5]]]])      
    end
    
    it "should not be able to contain local assignments" do
      expect { parser.parse "module Foo x = 5 end" }.to raise_error ParseError
    end
    
    it "should be able to contain class definitions" do
      expect( parser.parse( "module F class Y end end" ) ).to eq [[:module, :F, [[:class, :Y, nil, []]]]]
    end

    it "should be able to contain module definitions" do
      expect( parser.parse( "module F module Y end end" ) ).to eq [[:module, :F, [[:module, :Y, []]]]]
    end

  end
  
  describe "comments" do
    it "should parse empty comments" do
      expect( parser.parse( "#" ) ).to eq([])
    end
    it "should parse multiple lines of comments" do
      expect( parser.parse( "#bar\n#foo" ) ).to eq([])      
    end
    it "should parse multiple lines of empty comments" do
      expect( parser.parse( "#\n#" ) ).to eq([])      
    end
    it "should parse empty comments after nonempty ones" do
      expect( parser.parse( "# foo\n#" ) ).to eq([])            
    end
    it "should allow space before the comment sign" do
      expect( parser.parse( " # foo\n #" ) ).to eq([])                  
    end
  end
  
end