#include "c6502/c6502.h"

namespace c6502
{
std::ostream& operator<<(std::ostream& os, Cpu const& cpu)
{
    os << cpu.toString();
    return os;
}

std::string Cpu::toString() const
{
    std::stringstream ss;
    ss << "PC: 0x" << std::hex << unsigned(PC) << '\n'
       << "SP: 0x" << std::hex << unsigned(SP) << '\n'
       << "A:  0x" << std::hex << unsigned(A) << '\n'
       << "X:  0x" << std::hex << unsigned(X) << '\n'
       << "Y:  0x" << std::hex << unsigned(Y) << '\n'
       << "SR: 0b" << std::hex << unsigned(SR);

    return ss.str();
}

void Cpu::reset(Memory& memory)
{
    std::cout << "-- CPU reset --" << std::endl;
    memory.initialize();

    PC = c_reset_vector;
    SP = c_stack_top;

    A = 0;
    X = 0;
    Y = 0;

    SR = 0;
}

u8 Cpu::fetchByte(s32& cycles, const Memory& memory)
{
    const u8 data = memory[PC];
    PC++;
    cycles--;

    std::cout << "Fetch: " << std::hex << unsigned(data) << std::endl;

    return data;
}

u16 Cpu::fetchWord(s32& cycles, const Memory& memory)
{
    const u8 lowByte = fetchByte(cycles, memory);
    const u8 highByte = fetchByte(cycles, memory);
    const u16 data = (highByte << 8) | lowByte;

    std::cout << "Fetch: " << std::hex << unsigned(data) << std::endl;

    return data;
}

u8 Cpu::readByte(s32& cycles, const u16 address, const Memory& memory)
{
    const u8 data = memory[address];
    cycles--;

    std::cout << "Read:  " << std::hex << unsigned(data) << std::endl;

    return data;
}

u16 Cpu::readWord(s32& cycles, const u16 address, const Memory& memory)
{
    const u8 lowByte = readByte(cycles, address, memory);
    const u8 highByte = readByte(cycles, address + 1, memory);
    const u16 data = (highByte << 8) | lowByte;

    std::cout << "Read:  " << std::hex << unsigned(data) << std::endl;

    return data;
}

void Cpu::loadIntoRegister(u8& reg, const u8 value, const u8& zeroFlagReg)
{
    reg = value;
    Z = (zeroFlagReg == 0);
    N = (reg & (1 << 7)) != 0;
}

void Cpu::loadImmediate(s32& cycles, Memory& memory, u8& reg)
{
    const u8 value = fetchByte(cycles, memory);
    loadIntoRegister(reg, value, reg);
}

void Cpu::loadZeroPage(s32& cycles, Memory& memory, u8& reg)
{
    const u8 ZPAddr = fetchByte(cycles, memory);
    const u8 value = readByte(cycles, ZPAddr, memory);
    loadIntoRegister(reg, value, reg);
}

void Cpu::loadZeroPageOffset(s32& cycles, Memory& memory, u8& reg, u8& offsetReg)
{
    const u8 ZPAddr = fetchByte(cycles, memory);

    /// Should handle wrap around automatically since both are u8's
    const u8 ZPAddrWithOffset = ZPAddr + offsetReg;
    cycles--;

    const u8 value = readByte(cycles, ZPAddrWithOffset, memory);
    loadIntoRegister(reg, value, reg);
}

void Cpu::loadAbsolute(s32& cycles, Memory& memory, u8& reg, const u8 offset)
{
    const u16 absoluteAddr = fetchWord(cycles, memory);
    const u16 absoluteAddrWithOffset = absoluteAddr + offset;

    const bool crossedPageBoundary = (absoluteAddr & 0xFF00) != (absoluteAddrWithOffset & 0xFF00);
    if (crossedPageBoundary)
    {
        cycles--;
    }

    const u8 value = readByte(cycles, absoluteAddrWithOffset, memory);
    loadIntoRegister(reg, value, reg);
}

void Cpu::executeInstruction(const OP opCode, s32& cycles, Memory& memory)
{
    std::cout << "Ins:   " << OpCodeToString(opCode) << '\n';
    switch (opCode)
    {
        case OP::LDA_IM:
        {
            loadImmediate(cycles, memory, A);
            break;
        }
        case OP::LDA_ZP:
        {
            loadZeroPage(cycles, memory, A);
            break;
        }
        case OP::LDA_ZPX:
        {
            loadZeroPageOffset(cycles, memory, A, X);
            break;
        }
        case OP::LDA_ABS:
        {
            loadAbsolute(cycles, memory, A);
            break;
        }
        case OP::LDX_IM:
        {
            loadImmediate(cycles, memory, X);
            break;
        }
        case OP::LDX_ZP:
        {
            loadZeroPage(cycles, memory, X);
            break;
        }
        case OP::LDX_ZPY:
        {
            loadZeroPageOffset(cycles, memory, X, Y);
            break;
        }
        case OP::LDX_ABS:
        {
            loadAbsolute(cycles, memory, X);
            break;
        }
        case OP::LDY_IM:
        {
            loadImmediate(cycles, memory, Y);
            break;
        }
        case OP::LDY_ZP:
        {
            loadZeroPage(cycles, memory, Y);
            break;
        }
        case OP::LDY_ZPX:
        {
            loadZeroPageOffset(cycles, memory, Y, X);
            break;
        }
        case OP::LDY_ABS:
        {
            loadAbsolute(cycles, memory, Y);
            break;
        }
        case OP::TXS:
        {
            SP = X;
            cycles--;
            break;
        }
        case OP::NOP:
        {
            cycles--;
            break;
        }
        default:
            throw InvalidOpCode(opCode);
    }
}

s32 Cpu::execute(s32 cycles, Memory& memory)
{
    const s32 requestedCycles = cycles;

    while (cycles > 0)
    {
        // Fetch instruction from memory
        const u8 byte = fetchByte(cycles, memory);
        const auto ins = static_cast<OP>(byte);

        executeInstruction(ins, cycles, memory);
    }

    const s32 executedCycles = requestedCycles - cycles;
    return executedCycles;
}

void Cpu::executeInfinite(Memory& memory)
{
    while (true)
    {
        s32 dummy = 0xFF;

        // Fetch instruction from memory
        const u8 byte = fetchByte(dummy, memory);
        const auto ins = static_cast<OP>(byte);

        executeInstruction(ins, dummy, memory);
    }
}

} // namespace c6502
