require 'digest'

module Translator extend self

  def name(code)
    "i" + ((Digest::SHA1.base64digest code.to_s)[1..8].gsub /(\+|\/)/, "_")
  end

  def sub_callables!(tree)
    callables = {}
    code_segments = {:lambda => 2, :function => 3, :method => 3}
    tree.each do |node|
      if node.respond_to? :each
        sub_callables!(node).each do |cbl_id, cbl|
          callables[cbl_id] = cbl
        end
        if code_segments.has_key? node.first
          cbl_id = name node[code_segments[node.first]]
          code = node[code_segments[node.first]]
          # On the off chance that there is a hash function collision, issue a warning
          warn "Function Collision" if callables[cbl_id] && callables[cbl_id] != code
          callables[cbl_id] = code
          node[code_segments[node.first]] = cbl_id
        end
      end
    end
    callables
  end

  def sub_operators!(tree)
    binary_operators = {:+ => :__add__, :- => :__sub__, :* => :__mult__, :/ => :__div__, :== => :__eq__, :< => :__lt__, :> => :__gt__, :<= => :__leq__, :>= => :__geq__}
    tree.each do |node|
      if node.respond_to? :each
        sub_operators! node
        if binary_operators.has_key? node.first
          # This is a binary operator node
          op = node[0]
          op_left = node[1]
          op_right = node[2]
          # Replace operator with call
          node[0] = :"()"
          node[1] = [:".", op_left, binary_operators[op]]
          node[2] = [op_right]
        end
      end
    end
  end

  def translate!(tree)
    sub_operators! tree
    callables = sub_callables! tree
    return {callables: callables, tree: tree}
  end

end
