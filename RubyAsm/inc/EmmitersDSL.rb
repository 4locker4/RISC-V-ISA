
# =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=  ADDRESS FOR OFFSET.BASE =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= 
class Address
    attr_reader :offset, :base
  
    def initialize(offset, base)
        @offset = offset
        @base   = base
    end
end

# =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= EXTENDING INTEGER FOR SYNTAX: 8.x3 =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
class Integer
    def method_missing(name, *args)
        if args.empty? && name.to_s.match?(/^x(\d+)$/)
            match_result = name.to_s.match(/^x(\d+)$/)
            reg_num = match_result[1].to_i
            return Address.new(self, reg_num) if (0..31).include?(reg_num)
        end

        super
    end
  
    def respond_to_missing?(name, include_private = false)
        name.to_s.match?(/^x\d+$/) || super
    end
end

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= START ASSEMBLER =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+++=+=+=
class Assembler
    attr_reader :code

    def initialize
        @code = ''.b
        @labels = {}
        @pending_fixups = []
        @current_address = 0

        (0..31).each do |i|                                 # Creating registers
            self.class.send(:define_method, "x#{i}") { i }
        end
    end

#=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= START EMMITERS +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

    INSTRUCTIONS = {
        # r-types instructions
        add: { proc: ->(rd, rs, rt) { emit_r_type(rd, rs, rt, 0b011010) } },
        sub: { proc: ->(rd, rs, rt) { emit_r_type(rd, rs, rt, 0b011111) } },
        movz: { proc: ->(rd, rs, rt) { emit_r_type(rd, rs, rt, 0b000100) } },
        selc: { proc: ->(rd, rs1, rs2) { emit_r_type(rd, rs1, rs2, 0b000011) } },
        rbit: { proc: ->(rd, rs) { emit_r_type(rd, rs, 0, 0b011101) } },

        # i-types instructions
        st: { proc: ->(rt, addr) { emit_i_type_ld_st(rt, addr, 0b100101) } },
        ld: { proc: ->(rt, addr) { emit_i_type_ld_st(rt, addr, 0b100011) } },
        stp: { proc: ->(rt1, rt2, addr) { emit_i_stp(rt1, rt2, addr, 0b111001) } },
        beq: { proc: ->(rs, rt, offset) { emit_i_beq(rs, rt, offset, 0b010011) } },
        slti: { proc: ->(rs, rt, imm) { emit_i_type_slti(rs, rt, imm, 0b111011) } },
        usat: { proc: ->(rd, rs, imm5) { emit_i_type_rori_usat(rd, rs, imm5, 0b100010) } },
        rori: { proc: ->(rd, rs, imm5) { emit_i_type_rori_usat(rd, rs, imm5, 0b001100) } },

        # j-types instructions
        j: { proc: ->(addr) { emit_j_type(addr, 0b010110) } },

        # syscall
        syscall: { proc: -> { emit_syscall(0b011001) } }
    }.freeze

    def emit_r_type(rd, rs, rt, funct)
        validate_reg(rd)
        validate_reg(rs)
        validate_reg(rt) if rt != 0
      
        instr = (
            (0 << 26)  |
            (rs << 21) |
            (rt << 16) |
            (rd << 11) | 
            (0 << 6)   |
            funct
        )
        
        puts "R-Type Instruction:"
        puts "funct:     #{funct.to_s(2).rjust(6, '0')} (#{funct})"
        puts "rs:        #{rs.to_s(2).rjust(5, '0')} (#{rs})"
        puts "rt:        #{rt.to_s(2).rjust(5, '0')} (#{rt})"
        puts "rd:        #{rd.to_s(2).rjust(5, '0')} (#{rd})"
        puts "bytes:     #{[instr].pack('V').bytes.map { |b| "%08b" % b }.join(' ')}"
        puts "hex:       0x#{instr.to_s(16).rjust(8, '0').upcase}"
        puts "-" * 40

        @current_address += 4;
        [instr].pack('V')
    end

    def emit_i_beq(rs, rt, offset, opcode)
        validate_reg(rt)
        validate_reg(rs)
        unless (offset % 4) == 0
            raise ArgumentError, "BEQ offset must be word-aligned (multiple of 4), got #{offset}"
        end
        
        imm16 = encode_immediate(offset, 16, signed: true)

    # Create binary
        instr = (
        (opcode << 26) |        # opcode
        (rs     << 21) |        # rs
        (rt     << 16) |        # rt
        (imm16 & 0xFFFF)        # offset
        )

        puts "I-Type Instruction BEQ:"
        puts "opcode:    #{opcode.to_s(2).rjust(6, '0')} (#{opcode})"
        puts "rs:        #{rs.to_s(2).rjust(5, '0')} (#{rs})"
        puts "rt:        #{rt.to_s(2).rjust(5, '0')} (#{rt})"
        puts "imm16:     #{imm16.to_s(2).rjust(16, '0')} (#{imm16})"
        puts "bytes:     #{[instr].pack('V').bytes.map { |b| "%08b" % b }.join(' ')}"
        puts "hex:       0x#{instr.to_s(16).rjust(8, '0').upcase}"
        puts "-" * 40

        @current_address += 4;
        [instr].pack('V')
    end

    def emit_i_stp(rt1, rt2, address, opcode)
        validate_reg(rt1)
        validate_reg(rt2)
        validate_reg(address.base)

        imm11 = encode_immediate(address.offset, 11, signed: true)

    # Create binary
        instr = (
            (opcode << 26)       |  # opcode
            (address.base << 21) |  # base reg
            (rt1 << 16)          |  # rt1
            (rt2 << 11)          |  # rt2
            (imm11 & 0x7FF)         # offset
        )

        puts "I-Type Instruction STP:"
        puts "opcode:    #{opcode.to_s(2).rjust(6, '0')} (#{opcode})"
        puts "addr.base: #{address.base.to_s(2).rjust(5, '0')} (#{address.base})"
        puts "rt1:       #{rt1.to_s(2).rjust(5, '0')} (#{rt1})"
        puts "rt2:       #{rt2.to_s(2).rjust(5, '0')} (#{rt2})"
        puts "imm11:     #{imm11.to_s(2).rjust(11, '0')} (#{imm11})"
        puts "bytes:     #{[instr].pack('V').bytes.map { |b| "%08b" % b }.join(' ')}"
        puts "hex:       0x#{instr.to_s(16).rjust(8, '0').upcase}"
        puts "-" * 40

        @current_address += 4;
        [instr].pack('V')

    end

    def emit_i_type_ld_st(rt, address, opcode)
        validate_reg(rt)
        validate_reg(address.base)
      
        unless address.offset % 4 == 0
            raise ArgumentError, "Offset must be word-aligned (multiple of 4), got #{address.offset}"
        end
      
        imm16 = encode_immediate(address.offset, 16, signed: true)
      
    # Create binary
        instr = (
            (opcode << 26)       |
            (address.base << 21) |
            (rt << 16)           |
            (imm16 & 0xFFFF)
        )

        puts "I-Type Instruction SLTI or LD:"
        puts "opcode:    #{opcode.to_s(2).rjust(6, '0')} (#{opcode})"
        puts "addr.base: #{address.base.to_s(2).rjust(5, '0')} (#{address.base})"
        puts "rt:        #{rt.to_s(2).rjust(5, '0')} (#{rt})"
        puts "imm16:     #{imm16.to_s(2).rjust(16, '0')} (#{imm16})"
        puts "bytes:     #{[instr].pack('V').bytes.map { |b| "%08b" % b }.join(' ')}"
        puts "hex:       0x#{instr.to_s(16).rjust(8, '0').upcase}"
        puts "-" * 40
      
        @current_address += 4;
        [instr].pack('V')
    end

    def emit_i_type_slti(rs, rt, imm, opcode)
        validate_reg(rs)
        validate_reg(rt)
        imm16 = encode_immediate(imm, 16, signed: true)
      
    # Create binary
    instr = (
            (opcode << 26)  |
            (rs << 21)      |
            (rt << 16)      |
            (imm16 & 0xFFFF)
        )   

        puts "I-Type Instruction SLTI:"
        puts "opcode:    #{opcode.to_s(2).rjust(6, '0')} (#{opcode})"
        puts "rs:        #{rs.to_s(2).rjust(5, '0')} (#{rs})"
        puts "rt:        #{rt.to_s(2).rjust(5, '0')} (#{rt})"
        puts "imm16:     #{imm16.to_s(2).rjust(16, '0')} (#{imm16})"
        puts "bytes:     #{[instr].pack('V').bytes.map { |b| "%08b" % b }.join(' ')}"
        puts "hex:       0x#{instr.to_s(16).rjust(8, '0').upcase}"
        puts "-" * 40
      
        @current_address += 4;
        [instr].pack('V')
    end
    
    def emit_i_type_rori_usat(rd, rs, imm5, opcode)
        validate_reg(rd)
        validate_reg(rs)
      
        if imm5 < 0 || imm5 > 31
            raise ArgumentError, "Imm5 must be 0..31, got #{imm5}"
        end
      
    # Create binary
        instr = (
            (opcode << 26)  |
            (rd << 21)      |
            (rs << 16)      |
            (imm5 << 11)    |
            0               # Zeros!
        )
      
        puts "I-Type Instruction rori or usat:"
        puts "opcode:    #{opcode.to_s(2).rjust(6, '0')} (#{opcode})"
        puts "rd:        #{rd.to_s(2).rjust(5, '0')} (#{rd})"
        puts "rs:        #{rs.to_s(2).rjust(5, '0')} (#{rs})"
        puts "imm5:      #{imm5.to_s(2).rjust(5, '0')} (#{imm5})"
        puts "bytes:     #{[instr].pack('V').bytes.map { |b| "%08b" % b }.join(' ')}"
        puts "hex:       0x#{instr.to_s(16).rjust(8, '0').upcase}"
        puts "-" * 40

        @current_address += 4;
        [instr].pack('V')
    end

    def emit_j_type(address, opcode)
        unless address % 4 == 0
            raise ArgumentError, "J target must be word-aligned (multiple of 4), got #{address}"
        end
      
        imm26 = encode_immediate(address >> 2, 26, signed: false)

        puts "j target addr: #{imm26}"
    # Create binary
        instr = (opcode << 26) |    # opcode
                (imm26 & 0x3FFFFFF) # address

        puts "J-Type Instruction:"
        puts "opcode:    #{opcode.to_s(2).rjust(6, '0')} (#{opcode})"
        puts "imm26:     #{imm26.to_s(2).rjust(26, '0')} (#{imm26})"
        puts "bytes:     #{[instr].pack('V').bytes.map { |b| "%08b" % b }.join(' ')}"
        puts "hex:       0x#{instr.to_s(16).rjust(8, '0').upcase}"
        puts "-" * 40

        @current_address += 4;
        [instr].pack('V')
    end

    def emit_syscall(opcode)
        @current_address += 4;
        [opcode].pack('V')          # Create binary
    end
#+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= START SUPPORTING FUNCTS +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

    private

# Checking validity of num
    def encode_immediate(num, bits, signed: false)
        mask = (1 << bits) - 1                      # In max pos there is sign bit
        if signed
            max_pos = (1 << (bits - 1)) - 1         # Max 2 ^ (bits - 1) - 1, because of interp
            min_neg = -(1 << (bits - 1))            # Max neg 2 ^ (bits - 1)
            unless (min_neg..max_pos).include?(num)
                raise ArgumentError, "Immediate #{num} out of range for #{bits}-bit signed"
            end
            num & mask
        else
            unless (0..mask).include?(num)
                raise ArgumentError, "Immediate #{num} out of range for #{bits}-bit unsigned"
            end
            num
        end
    end

# Checking validity of register
    def validate_reg(r)
        raise ArgumentError, "Register must be 1..30, got #{r}"unless r.is_a?(Integer) && (0..31).include?(r)
    end

#+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= DISPATCHER +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

    def method_missing(name, *args)
        if args.empty? && !INSTRUCTIONS.key?(name)
            @labels[name] = @current_address
            apply_pending_labels(name)
            return
        end
        instr = INSTRUCTIONS[name]
        if instr
            begin
                processed_args = args.map do |arg|
                    if arg.is_a?(Symbol) && @labels.key?(arg)
                        @labels[arg]
                    elsif arg.is_a?(String) && @labels.key?(arg.to_sym)
                        @labels[arg.to_sym]
                    else
                        arg
                    end
                end
            
                case name
                when :j
                    handle_jmp(*processed_args)
                when :beq
                    handle_branch(*processed_args)
                else
                    binary = instance_exec(*processed_args, &instr[:proc])
                    @code << binary
                    binary
                end
            rescue ArgumentError => e
                raise ArgumentError, "#{name} failed: #{e.message}"
            end
        else
            super
        end
    end

    def handle_jmp (target)
        if target.is_a?(Symbol)
            @pending_fixups << {
                type: :j,
                address: @current_address,
                target_label: target
            }

            instr = ((0b010110 << 26) | 0)
            @code << [instr].pack('V')
            @current_address += 4
        else
            @code << emit_j_type(target, 0b010110)
        end
    end

    def handle_branch(rs, rt, target)
        if target.is_a?(Symbol)
            @pending_fixups << {
                type: :branch,
                address: @current_address,
                rs: rs,
                rt: rt,
                target_label: target
            }
            instr = (
                (0b010011 << 26) | 
                (rs       << 21) | 
                (rt       << 16) | 
                0)
                @code << [instr].pack('V')
                @current_address += 4
        else
            @code << emit_i_beq(rs, rt, target - (@current_address + 4), 0b010011)
        end
    end
    
    def apply_pending_labels(label_name)
        @pending_fixups.reject! do |fixup|
            if fixup[:target_label] == label_name
                target_addr = @labels[label_name]
    
                case fixup[:type]
                when :j
                    instr = ((0b010110 << 26) | 
                            ((target_addr >> 2) & 0x3FFFFFF))
                    @code[fixup[:address], 4] = [instr].pack('V')
                when :branch
                    offset = target_addr - (fixup[:address] + 4)
                    imm16 = encode_immediate(offset, 16, signed: true)
                    puts "Imm apply: #{imm16} #{label_name}"
                    instr = ((0b010011  << 26) |
                            (fixup[:rs] << 21) | 
                            (fixup[:rt] << 16) | 
                            (imm16 & 0xFFFF))
                    @code[fixup[:address], 4] = [instr].pack('V')
                end
                true
            else
                false
            end
        end
    end

    def resolve_labels
        @pending_fixups.each do |fixup|
            target_addr = @labels[fixup[:target_label]]
            if target_addr.nil?
                raise ArgumentError, "Label #{fixup[:target_label]} not found"
            end
    
            case fixup[:type]
            when :j
                instr = (
                    (0b010110 << 26) | 
                    ((target_addr >> 2) & 0x3FFFFFF))
                @code[fixup[:address], 4] = [instr].pack('V')
            when :branch
                offset = target_addr - (fixup[:address] + 4)
                imm16 = encode_immediate(offset, 16, signed: true)
                puts "Imm resolve: #{imm16} #{fixup[:label_name]}"
                instr = ((0b010011  << 26) |
                        (fixup[:rs] << 21) |
                        (fixup[:rt] << 16) |
                        (imm16 & 0xFFFF))
                @code[fixup[:address], 4] = [instr].pack('V')
            end
        end
    end
    
    def get_code
        resolve_labels
        @code
    end

    public :get_code

    def respond_to_missing?(name, include_private = false)
        name.to_s.end_with?(':') || INSTRUCTIONS.key?(name) || super
    end
end

#+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= WRAPPING +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

def assemble(&block)
    asm = Assembler.new
    asm.instance_eval(&block)
    asm.get_code
end