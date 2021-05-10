#include "test_c6502.h"

namespace c6502
{
class CpuFixtureInsLoad : public CpuFixture
{
public:
    CpuFixtureInsLoad()
    {
    }

    /// Checks that the flags are set correctly for load instructions
    void requireLoadState(const u8 Cpu::*reg)
    {
        cpuCopy.Z = (cpuCopy.*reg == 0x00);
        cpuCopy.N = (cpuCopy.*reg & 0b1000'0000) > 0;

        requireState();
    }

    void testLoadImmediate(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("Constant is placed after instruction")
        {
            const u8 data = GENERATE(0x00, 0x42, 0xFF);

            memory[0x1000] = opCode;
            memory[0x1001] = data;

            const s32 cyclesExpected = 2;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute();

                THEN("Constant is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = data;

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireLoadState(reg);
                }
            }
        }
    }

    void testLoadZeroPage(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("ZeroPage address is placed after instruction")
        {
            const u16 zeroPageAddr = 0x0037;
            const u8 data = GENERATE(0x00, 0x42, 0xFF);

            memory[0x1000] = opCode;
            memory[0x1001] = zeroPageAddr;
            memory[zeroPageAddr] = data;

            const s32 cyclesExpected = 3;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute();

                THEN("ZeroPage data is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = data;

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireLoadState(reg);
                }
            }
        }
    }

    void testLoadZeroPageOffset(const Cpu::OP opCode, u8 Cpu::*reg, u8 Cpu::*offsetReg)
    {
        GIVEN("ZeroPage address is placed after instruction and offset register is set")
        {
            cpu.*offsetReg = GENERATE(0x01, 0xFF);
            const u16 zeroPageAddr = 0x0037;

            /// Handles zero page wrap around
            const u16 zeroPageAddrWithOffset = (zeroPageAddr + cpu.*offsetReg) & 0x00FF;
            const u8 data = GENERATE(0x00, 0x42, 0xFF);

            memory[0x1000] = opCode;
            memory[0x1001] = zeroPageAddr;
            memory[zeroPageAddrWithOffset] = data;

            const s32 cyclesExpected = 4;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute();

                THEN("ZeroPage + offset addressed data is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = data;

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireLoadState(reg);
                }
            }
        }
    }

    void testLoadAbsolute(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("Absolute address is placed after instruction")
        {
            const u16 absoluteAddr = 0xABCD;
            const u8 data = GENERATE(0x00, 0x42, 0xFF);

            memory[0x1000] = opCode;
            memory[0x1001] = absoluteAddr & 0x00FF;
            memory[0x1002] = (absoluteAddr >> 8) & 0x00FF;
            memory[absoluteAddr] = data;

            const s32 PCIncrementsExpected = 3;
            const s32 cyclesExpected = 4;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute();

                THEN("Absolute addressed data is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = data;

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireLoadState(reg);
                }
            }
        }
    }

    void testLoadAbsoluteOffset(const Cpu::OP opCode, u8 Cpu::*reg, u8 Cpu::*offsetReg)
    {
        GIVEN("Absolute address is placed after instruction and offset register is set")
        {
            cpu.*offsetReg = GENERATE(0x00, 0x01, 0xFF);
            const u16 absoluteAddr = 0xABCD;
            const u16 effectiveAddr = absoluteAddr + cpu.*offsetReg;
            const u8 data = GENERATE(0x00, 0x42, 0xFF);

            memory[0x1000] = opCode;
            memory[0x1001] = absoluteAddr & 0x00FF;
            memory[0x1002] = absoluteAddr >> 8;
            memory[effectiveAddr] = data;

            const bool crossedPageBoundary = (absoluteAddr & 0xFF00) != (effectiveAddr & 0xFF00);

            const s32 PCIncrementsExpected = 3;
            const s32 cyclesExpected = crossedPageBoundary ? 5 : 4;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute();

                THEN("Absolute + offset addressed data is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = data;

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireLoadState(reg);
                }
            }
        }
    }

    void testLoadIndexedIndirect(const Cpu::OP opCode, u8 Cpu::*offsetReg)
    {
        GIVEN(
            "Indirect address with offset is placed after instruction, offset register is set, "
            "effective address is set and effective address has value")
        {
            cpu.*offsetReg = GENERATE(0x00, 0x01, 0xFF);
            const u8 ZPAddr = GENERATE(0x12, 0xFF);

            /// Handles zero page wrap around
            const u16 indirectAddr = (ZPAddr + cpu.*offsetReg) & 0x00FF;
            const u16 effectiveAddr = 0xABCD;
            const u8 data = 0x42;

            memory[0x1000] = opCode;
            memory[0x1001] = ZPAddr & 0x00FF;
            memory[indirectAddr] = effectiveAddr & 0x00FF;
            memory[indirectAddr + 1] = effectiveAddr >> 8;
            memory[effectiveAddr] = data;

            const s32 PCIncrementsExpected = 2;
            const s32 cyclesExpected = 6;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute();

                THEN("Indirect + offset addressed data is loaded into the A register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.A = data;

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireLoadState(&Cpu::A);
                }
            }
        }
    }

    void testLoadIndirectIndexed(const Cpu::OP opCode, u8 Cpu::*offsetReg)
    {
        GIVEN(
            "Indirect address is placed after instruction, offset register is set, "
            "effective address is set and effective address with offset has value")
        {
            cpu.*offsetReg = GENERATE(0x00, 0x01, 0xFF);
            const u8 ZPAddr = GENERATE(0x12, 0xFF);

            const u16 indirectAddr = 0xABCD;
            const u16 effectiveAddr = indirectAddr + cpu.*offsetReg;
            const u8 data = 0x42;

            memory[0x1000] = opCode;
            memory[0x1001] = ZPAddr;
            memory[ZPAddr] = indirectAddr & 0x00FF;
            memory[ZPAddr + 1] = indirectAddr >> 8;
            memory[effectiveAddr] = data;

            const bool crossedPageBoundary = (effectiveAddr & 0xFF00) != (indirectAddr & 0xFF00);

            const s32 PCIncrementsExpected = 2;
            const s32 cyclesExpected = crossedPageBoundary ? 6 : 5;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute();

                THEN("Indirect + offset addressed data is loaded into the A register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.A = data;

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireLoadState(&Cpu::A);
                }
            }
        }
    }
};

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_IM")
{
    testLoadImmediate(Cpu::OP::LDA_IM, &Cpu::A);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_ZP")
{
    testLoadZeroPage(Cpu::OP::LDA_ZP, &Cpu::A);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_ZPX")
{
    testLoadZeroPageOffset(Cpu::OP::LDA_ZPX, &Cpu::A, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_ABS")
{
    testLoadAbsolute(Cpu::OP::LDA_ABS, &Cpu::A);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_ABSX")
{
    testLoadAbsoluteOffset(Cpu::OP::LDA_ABSX, &Cpu::A, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_ABSY")
{
    testLoadAbsoluteOffset(Cpu::OP::LDA_ABSY, &Cpu::A, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_IND_ZPX")
{
    testLoadIndexedIndirect(Cpu::OP::LDA_IND_ZPX, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA_IND_ZPY")
{
    testLoadIndirectIndexed(Cpu::OP::LDA_IND_ZPY, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDX")
{
    testLoadImmediate(Cpu::OP::LDX_IM, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDX_ZP")
{
    testLoadZeroPage(Cpu::OP::LDX_ZP, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDX_ZPY")
{
    testLoadZeroPageOffset(Cpu::OP::LDX_ZPY, &Cpu::X, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDX_ABS")
{
    testLoadAbsolute(Cpu::OP::LDX_ABS, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDX_ABSY")
{
    testLoadAbsoluteOffset(Cpu::OP::LDX_ABSY, &Cpu::X, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDY_IM")
{
    testLoadImmediate(Cpu::OP::LDY_IM, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDY_ZP")
{
    testLoadZeroPage(Cpu::OP::LDY_ZP, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDY_ZPX")
{
    testLoadZeroPageOffset(Cpu::OP::LDY_ZPX, &Cpu::Y, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDY_ABS")
{
    testLoadAbsolute(Cpu::OP::LDY_ABS, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDY_ABSX")
{
    testLoadAbsoluteOffset(Cpu::OP::LDY_ABSX, &Cpu::Y, &Cpu::X);
}

} // namespace c6502
