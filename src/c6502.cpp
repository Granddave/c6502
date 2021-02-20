#include "c6502/c6502.h"

#include <bitset>
#include <iomanip>

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
    ss << "PC: 0x" << std::setfill('0') << std::setw(4) << std::hex << unsigned(PC) << ", ";
    ss << "SP: 0x" << std::setfill('0') << std::setw(2) << std::hex << unsigned(SP) << ", ";
    ss << "A: 0x" << std::setfill('0') << std::setw(2) << std::hex << unsigned(A) << ", ";
    ss << "X: 0x" << std::setfill('0') << std::setw(2) << std::hex << unsigned(X) << ", ";
    ss << "Y: 0x" << std::setfill('0') << std::setw(2) << std::hex << unsigned(Y) << ", ";
    ss << "SR: 0b" << std::bitset<8>(SR).to_string();

    return ss.str();
}

void Cpu::reset(Memory& memory, const u16 startAddr)
{
    std::cout << "-- CPU reset --" << std::endl;
    memory.initialize();

    memory[c_reset_vector] = startAddr & 0xFF;
    memory[c_reset_vector + 1] = startAddr << 8;

    PC = startAddr;
    SP = c_stack_top;

    A = 0;
    X = 0;
    Y = 0;

    SR = 0;
}

u8 Cpu::fetchByte(s32& cycles, const Memory& memory, const bool log)
{
    const u8 data = memory[PC];
    if (log)
    {
        std::cout << "FetchB: " << std::hex << unsigned(PC) << ": " << std::hex << unsigned(data)
                  << std::endl;
    }

    PC++;
    cycles--;

    return data;
}

u16 Cpu::fetchWord(s32& cycles, const Memory& memory)
{
    const bool log = false;
    const u8 lowByte = fetchByte(cycles, memory, log);
    const u8 highByte = fetchByte(cycles, memory, log);
    const u16 data = (highByte << 8) | lowByte;

    std::cout << "FetchW: " << std::hex << unsigned(PC) << "+1: " << std::hex << unsigned(data)
              << std::endl;

    return data;
}

u8 Cpu::readByte(s32& cycles, const u16 address, const Memory& memory, const bool log)
{
    const u8 data = memory[address];
    if (log)
    {
        std::cout << "ReadB : " << std::hex << unsigned(address) << ": " << std::hex
                  << unsigned(data) << std::endl;
    }
    cycles--;

    return data;
}

u16 Cpu::readWord(s32& cycles, const u16 address, const Memory& memory)
{
    const bool log = false;
    const u8 lowByte = readByte(cycles, address, memory, log);
    const u8 highByte = readByte(cycles, address + 1, memory, log);
    const u16 data = (highByte << 8) | lowByte;

    std::cout << "ReadW : " << std::hex << unsigned(address) << ": " << std::hex << unsigned(data)
              << std::endl;

    return data;
}

void Cpu::loadIntoRegister(u8& reg, const u8 value, const u8& zeroFlagReg)
{
    reg = value;
    Z = (zeroFlagReg == 0x00);
    N = (reg & 0b1000'0000) != 0;
}

void Cpu::loadIntoRegister(u8& reg, const u8 value)
{
    loadIntoRegister(reg, value, reg);
}

void Cpu::executeInstruction(const OP opCode, s32& cycles, Memory& memory)
{
    std::cout << "Ins   : " << OpCodeToString(opCode) << '\n';
    switch (opCode)
    {
        case OP::LDA_IM:
        {
            const u8 value = readImmediate(cycles, memory);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDA_ZP:
        {
            const u8 value = readZeroPage(cycles, memory);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDA_ZPX:
        {
            const u8 value = readZeroPageOffset(cycles, memory, X);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDA_ABS:
        {
            const u8 value = readAbsolute(cycles, memory);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDA_ABSX:
        {
            const u8 value = readAbsoluteOffset(cycles, memory, X);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDA_ABSY:
        {
            const u8 value = readAbsoluteOffset(cycles, memory, Y);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDA_IND_ZPX:
        {
            const u8 value = readZeroPageIndirectX(cycles, memory, X);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDA_IND_ZPY:
        {
            const u8 value = readZeroPageIndirectY(cycles, memory, Y);
            loadIntoRegister(A, value);
            break;
        }
        case OP::LDX_IM:
        {
            const u8 value = readImmediate(cycles, memory);
            loadIntoRegister(X, value);
            break;
        }
        case OP::LDX_ZP:
        {
            const u8 value = readZeroPage(cycles, memory);
            loadIntoRegister(X, value);
            break;
        }
        case OP::LDX_ZPY:
        {
            const u8 value = readZeroPageOffset(cycles, memory, Y);
            loadIntoRegister(X, value);
            break;
        }
        case OP::LDX_ABS:
        {
            const u8 value = readAbsolute(cycles, memory);
            loadIntoRegister(X, value);
            break;
        }
        case OP::LDX_ABSY:
        {
            const u8 value = readAbsoluteOffset(cycles, memory, Y);
            loadIntoRegister(X, value);
            break;
        }
        case OP::LDY_IM:
        {
            const u8 value = readImmediate(cycles, memory);
            loadIntoRegister(Y, value);
            break;
        }
        case OP::LDY_ZP:
        {
            const u8 value = readZeroPage(cycles, memory);
            loadIntoRegister(Y, value);
            break;
        }
        case OP::LDY_ZPX:
        {
            const u8 value = readZeroPageOffset(cycles, memory, X);
            loadIntoRegister(Y, value);
            break;
        }
        case OP::LDY_ABS:
        {
            const u8 value = readAbsolute(cycles, memory);
            loadIntoRegister(Y, value);
            break;
        }
        case OP::LDY_ABSX:
        {
            const u8 value = readAbsoluteOffset(cycles, memory, X);
            loadIntoRegister(Y, value);
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
        {
            throw InvalidOpCode(opCode);
        }
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
        s32 dummyCycles = 0xFF;

        // Fetch instruction from memory
        const u8 byte = fetchByte(dummyCycles, memory);
        const auto ins = static_cast<OP>(byte);

        executeInstruction(ins, dummyCycles, memory);
    }
}

} // namespace c6502
