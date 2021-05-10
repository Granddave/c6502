#include "c6502/c6502.h"

namespace c6502
{
u8 Cpu::readImmediate()
{
    return fetchByte();
}

u8 Cpu::readZeroPage()
{
    const u8 ZPAddr = fetchByte();
    return readByte(ZPAddr);
}

u8 Cpu::readZeroPageOffset(u8& offsetReg)
{
    const u8 ZPAddr = fetchByte();

    /// Should handle wrap around automatically since both are u8's
    const u8 ZPAddrWithOffset = ZPAddr + offsetReg;
    m_cycles++;

    return readByte(ZPAddrWithOffset);
}

u8 Cpu::readAbsolute()
{
    const u16 absoluteAddr = fetchWord();
    return readByte(absoluteAddr);
}

u8 Cpu::readAbsoluteOffset(u8& offsetReg)
{
    const u16 absoluteAddr = fetchWord();
    const u16 effectiveAddr = absoluteAddr + offsetReg;

    const bool crossedPageBoundary = (absoluteAddr & 0xFF00) != (effectiveAddr & 0xFF00);
    if (crossedPageBoundary)
    {
        m_cycles++;
    }

    return readByte(effectiveAddr);
}

u8 Cpu::readZeroPageIndirectX(const u8& offset)
{
    const u8 ZPAddr = fetchByte();
    const u8 indirectAddr = ZPAddr + offset;
    m_cycles++; // TODO: Is this placement correct?

    const u16 effectiveAddr = readWord(indirectAddr);

    return readByte(effectiveAddr);
}

u8 Cpu::readZeroPageIndirectY(const u8& offset, const bool alwaysAddExtraCycle)
{
    const u8 ZPAddr = fetchByte();
    const u16 indirectAddr = readWord(ZPAddr);
    const u16 effectiveAddr = indirectAddr + offset;

    const bool crossedPageBoundary = (indirectAddr & 0xFF00) != (effectiveAddr & 0xFF00);
    if (crossedPageBoundary || alwaysAddExtraCycle)
    {
        m_cycles++;
    }

    return readByte(effectiveAddr);
}

}; // namespace c6502
