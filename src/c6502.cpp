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

void Cpu::reset(const u16 startAddr)
{
    std::cout << "-- CPU reset --" << std::endl;

    m_memory.reset();
    m_memory.at(c_reset_vector) = startAddr & 0xFF;
    m_memory.at(c_reset_vector + 1) = startAddr << 8;

    PC = startAddr;
    SP = c_stack_top;

    A = 0;
    X = 0;
    Y = 0;

    SR = 0;
}

u8 Cpu::fetchByte(const bool log)
{
    const u8 data = m_memory.at(PC);
    if (log)
    {
        std::cout << "FetchB: " << std::hex << unsigned(PC) << ": " << std::hex << unsigned(data)
                  << std::endl;
    }

    PC++;
    m_cycles++;

    return data;
}

u16 Cpu::fetchWord()
{
    const bool log = false;
    const u8 lowByte = fetchByte(log);
    const u8 highByte = fetchByte(log);
    const u16 data = (highByte << 8) | lowByte;

    std::cout << "FetchW: " << std::hex << unsigned(PC) << "+1: " << std::hex << unsigned(data)
              << std::endl;

    return data;
}

u8 Cpu::readByte(const u16 address, const bool log)
{
    const u8 data = m_memory.at(address);
    if (log)
    {
        std::cout << "ReadB : " << std::hex << unsigned(address) << ": " << std::hex
                  << unsigned(data) << std::endl;
    }
    m_cycles++;

    return data;
}

u16 Cpu::readWord(const u16 address)
{
    const bool log = false;
    const u8 lowByte = readByte(address, log);
    const u8 highByte = readByte(address + 1,log);
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

void Cpu::executeInstruction(const OP opCode)
{
    std::cout << "Ins   : " << OpCodeToString(opCode) << '\n';
    switch (opCode)
    {
        case OP::LDA_IM:
        {
            const u8 value = readImmediate();
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDA_ZP:
        {
            const u8 value = readZeroPage();
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDA_ZPX:
        {
            const u8 value = readZeroPageOffset(X);
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDA_ABS:
        {
            const u8 value = readAbsolute();
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDA_ABSX:
        {
            const u8 value = readAbsoluteOffset(X);
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDA_ABSY:
        {
            const u8 value = readAbsoluteOffset(Y);
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDA_IND_ZPX:
        {
            const u8 value = readZeroPageIndirectX(X);
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDA_IND_ZPY:
        {
            const u8 value = readZeroPageIndirectY(Y);
            loadIntoRegister(A, value);
            return;
        }
        case OP::LDX_IM:
        {
            const u8 value = readImmediate();
            loadIntoRegister(X, value);
            return;
        }
        case OP::LDX_ZP:
        {
            const u8 value = readZeroPage();
            loadIntoRegister(X, value);
            return;
        }
        case OP::LDX_ZPY:
        {
            const u8 value = readZeroPageOffset(Y);
            loadIntoRegister(X, value);
            return;
        }
        case OP::LDX_ABS:
        {
            const u8 value = readAbsolute();
            loadIntoRegister(X, value);
            return;
        }
        case OP::LDX_ABSY:
        {
            const u8 value = readAbsoluteOffset(Y);
            loadIntoRegister(X, value);
            return;
        }
        case OP::LDY_IM:
        {
            const u8 value = readImmediate();
            loadIntoRegister(Y, value);
            return;
        }
        case OP::LDY_ZP:
        {
            const u8 value = readZeroPage();
            loadIntoRegister(Y, value);
            return;
        }
        case OP::LDY_ZPX:
        {
            const u8 value = readZeroPageOffset(X);
            loadIntoRegister(Y, value);
            return;
        }
        case OP::LDY_ABS:
        {
            const u8 value = readAbsolute();
            loadIntoRegister(Y, value);
            return;
        }
        case OP::LDY_ABSX:
        {
            const u8 value = readAbsoluteOffset(X);
            loadIntoRegister(Y, value);
            return;
        }
        case OP::TXS:
        {
            SP = X;
            m_cycles++;
            return;
        }
        case OP::NOP:
        {
            m_cycles++;
            return;
        }
        case OP::HALT:
        {
            // Don't register the fetch for this instruction
            m_cycles--;
            m_running = false;
            return;
        }
    }

    throw InvalidOpCode(opCode);
}

s32 Cpu::execute()
{
    m_cycles = 0;

    while (m_running)
    {
        executeInstruction(static_cast<OP>(fetchByte()));
    }

    return m_cycles;
}

} // namespace c6502
