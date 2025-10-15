require_relative '../inc/EmmitersDSL.rb'

module Kernel
  undef :syscall
end

# x1 - n_nums
# x2 = 1
# x3 - first fib num
# x4 - second fib num
# x5 - summator
# x6 - counter
# x7 = 0 - for cond
binary = assemble do
  ld x1, 0.x3
  ld x2, 4.x3
  add x4, x2, x3
  beq x1, x6, :end_fib
  start_fib
  add x5, x3, x4
  movz x3, x4, x7
  movz x4, x5, x7
  add x6, x6, x2
  beq x1, x6, :end_fib
  j :start_fib
  end_fib
  ld x7, 8.x6
  syscall
end

puts "Binary: #{binary.inspect}"
  
output_file = "../RubyAsm/outp.bin"
File.open(output_file, 'wb') do |f|
  f.write(binary)
end