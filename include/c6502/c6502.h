#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>

using u32 = std::uint32_t;
using s32 = std::int32_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;

namespace c6502
{
struct Memory
{
    /* The first 256 byte page of memory ($0000-$00FF) is referred to as 'Zero Page'
     * and is the focus of a number of special addressing modes that result in shorter
     * (and quicker) instructions or allow indirect access to the memory.
     *
     * The second page of memory ($0100-$01FF) is reserved for the system stack and
     * which cannot be relocated.
     */

    static constexpr std::uint32_t MEM_MAX = 64 * 1024;
    std::array<u8, MEM_MAX> data;

    u8& operator[](const std::size_t pos)
    {
        assert(pos < MEM_MAX);
        return data.at(pos);
    }

    u8 operator[](const std::size_t pos) const
    {
        assert(pos < MEM_MAX);
        return data.at(pos);
    }

    void initialize()
    {
        std::fill(std::begin(data), std::end(data), 0);
    }
};

inline bool operator==(const Memory& lhs, const Memory& rhs)
{
    return std::equal(lhs.data.begin(), lhs.data.end(), rhs.data.begin());
}

inline bool operator!=(const Memory& lhs, const Memory& rhs)
{
    return !(lhs == rhs);
}

class InvalidOpCode : public std::runtime_error
{
public:
    InvalidOpCode(const u8 opCode) : std::runtime_error("")
    {
        std::stringstream ss;
        ss << "OpCode not implemented: 0x" << std::hex << unsigned(opCode);
        m_errorMessage = ss.str();
    }

    virtual const char* what() const throw()
    {
        return m_errorMessage.c_str();
    }

private:
    std::string m_errorMessage;
};

struct Cpu
{
    static constexpr u16 c_nmi_vector = 0xFFFA;
    static constexpr u16 c_reset_vector = 0xFFFC;
    static constexpr u16 c_irq_vector = 0xFFFE;
    static constexpr u16 c_stack_top = 0xFF;

    /// OP Codes
    enum OP : u8
    {
        LDA_IM = 0xA9,
        TXS = 0x9A,
        NOP = 0xEA
    };

    std::string OpCodeToString(const u8 opCode);

    /* The value of program counter is modified automatically as instructions are executed.
     * The value of the program counter can be modified by executing a jump, a relative branch
     * or a subroutine call to another memory address or by returning from a subroutine or
     * interrupt. */
    u16 PC; // Program counter

    /* The processor supports a 256 byte stack located between $0100 and $01FF.
     * The stack pointer is an 8 bit register and holds the low 8 bits of the next free
     * location on the stack.
     * The location of the stack is fixed and cannot be moved.
     * Pushing bytes to the stack causes the stack pointer to be decremented.
     * Conversely pulling bytes causes it to be incremented. */
    u8 SP; // Stack pointer

    /// Registers
    u8 A; // Accumulator
    u8 X; // Index register X
    u8 Y; // Index register Y

    /// Processor status register flags
    union
    {
        struct
        {
            u8 C : 1; // Carry flag
            u8 Z : 1; // Zero flag
            u8 I : 1; // Interrupt disable
            u8 D : 1; // Decimal mode
            u8 B : 1; // Break command
            u8 U : 1; // Unused bit
            u8 O : 1; // Overflow flag
            u8 N : 1; // Negative flag
        };
        u8 SR;
    };

    /// Returns a string representation of the CPU's registers
    std::string toString() const;

    /// Resets the CPU and memory to their initialized state
    void reset(Memory& memory);

    /// Fetches a byte from specified address and increments the program counter
    u8 fetchByte(s32& cycles, const Memory& memory);

    /// Fetches a 16 bit word from specified address and increments the program counter
    u8 fetchword(s32& cycles, const Memory& memory);

    /// Reads a byte from address
    u8 readByte(s32& cycles, const u16 address, const Memory& memory);

    /// Reads a 16 bit word from address
    u16 readWord(s32& cycles, const u16 address, const Memory& memory);

    /// Executes an instruction
    void executeInstruction(const OP opCode, s32& cycles, Memory& memory);

    /// Executes n cycles
    s32 execute(s32 cycles, Memory& memory);
};

inline bool operator==(const Cpu& lhs, const Cpu& rhs)
{
    return (lhs.PC == rhs.PC && lhs.SP == rhs.SP && lhs.A == rhs.A && lhs.X == rhs.X &&
            lhs.Y == rhs.Y && lhs.SR == rhs.SR);
}

inline bool operator!=(const Cpu& lhs, const Cpu& rhs)
{
    return !(lhs == rhs);
}

} // namespace c6502
