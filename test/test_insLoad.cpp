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

            memory[0xFFFC] = opCode;
            memory[0xFFFD] = data;

            const s32 cyclesExpected = 2;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

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

            memory[0xFFFC] = opCode;
            memory[0xFFFD] = zeroPageAddr;
            memory[zeroPageAddr] = data;

            const s32 cyclesExpected = 3;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

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

            memory[0xFFFC] = opCode;
            memory[0xFFFD] = zeroPageAddr;
            memory[zeroPageAddrWithOffset] = data;

            const s32 cyclesExpected = 4;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

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

    void testLoadAbsolute(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("Absolute address is placed after instruction")
        {
            const u16 absoluteAddr = 0xABCD;
            const u8 data = GENERATE(0x00, 0x42, 0xFF);

            memory[0xFFFC] = opCode;
            memory[0xFFFD] = absoluteAddr & 0x00FF;
            memory[0xFFFE] = (absoluteAddr >> 8) & 0x00FF;
            memory[absoluteAddr] = data;

            const s32 PCIncrementsExpected = 3;
            const s32 cyclesExpected = 4;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

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
};

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDA")
{
    testLoadImmediate(Cpu::OP::LDA_IM, &Cpu::A);
    testLoadZeroPage(Cpu::OP::LDA_ZP, &Cpu::A);
    testLoadZeroPageOffset(Cpu::OP::LDA_ZPX, &Cpu::A, &Cpu::X);
    testLoadAbsolute(Cpu::OP::LDA_ABS, &Cpu::A);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDX")
{
    testLoadImmediate(Cpu::OP::LDX_IM, &Cpu::X);
    testLoadZeroPage(Cpu::OP::LDX_ZP, &Cpu::X);
    testLoadZeroPageOffset(Cpu::OP::LDX_ZPY, &Cpu::X, &Cpu::Y);
    testLoadAbsolute(Cpu::OP::LDX_ABS, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixtureInsLoad, "LDY")
{
    testLoadImmediate(Cpu::OP::LDY_IM, &Cpu::Y);
    testLoadZeroPage(Cpu::OP::LDY_ZP, &Cpu::Y);
    testLoadZeroPageOffset(Cpu::OP::LDY_ZPX, &Cpu::Y, &Cpu::X);
    testLoadAbsolute(Cpu::OP::LDY_ABS, &Cpu::Y);
}

} // namespace c6502
