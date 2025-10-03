require_relative '../inc/EmmitersDSL'

binary = assemble do
    add x1, x2, x3
    sub x4, x5, x6
    rbit x7, x8
    st x1, 8.x3
    stp x2, x3, (-4).x4
    beq x5, x6, 16
    j 0x1000
    slti x9, x10, 100
    usat x11, x12, 8
    ld x13, 12.x14
    rori x15, x16, 5
    syscall
  end
  
  puts "Binary:"
  puts binary.inspect