#include "c6502/c6502.h"

namespace c6502
{
u8 Cpu::readImmediate(s32& cycles, Memory& memory)
{
    return fetchByte(cycles, memory);
}

u8 Cpu::readZeroPage(s32& cycles, Memory& memory)
{
    const u8 ZPAddr = fetchByte(cycles, memory);
    return readByte(cycles, ZPAddr, memory);
}

u8 Cpu::readZeroPageOffset(s32& cycles, Memory& memory, u8& offsetReg)
{
    const u8 ZPAddr = fetchByte(cycles, memory);

    /// Should handle wrap around automatically since both are u8's
    const u8 ZPAddrWithOffset = ZPAddr + offsetReg;
    cycles--;

    return readByte(cycles, ZPAddrWithOffset, memory);
}

u8 Cpu::readAbsolute(s32& cycles, Memory& memory)
{
    const u16 absoluteAddr = fetchWord(cycles, memory);
    return readByte(cycles, absoluteAddr, memory);
}

}; // namespace c6502
