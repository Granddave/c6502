#include "catch2/catch.hpp"

#include "c6502/c6502.h"

namespace c6502
{
class CpuFixture
{
protected:
    Cpu cpu;
    Memory memory;

    Cpu cpuCopy;
    Memory memoryCopy;

public:
    CpuFixture()
    {
        cpu.reset(memory);
    }

    /// Takes a snapshot of the CPU and memory for later comparisons
    void takeSnapshot()
    {
        cpuCopy = cpu;
        memoryCopy = memory;
    }

    /// Checks that the current parts are the same as the modified copies
    void requireState() const
    {
        REQUIRE(memory == memoryCopy);
        REQUIRE(cpu == cpuCopy);
    }

    void testLoadImmediate(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("Constant is placed after instruction")
        {
            const u8 constant = GENERATE(0x00, 0x42, 0xFF);
            memory[0xFFFC] = opCode;
            memory[0xFFFD] = constant;
            const s32 cyclesExpected = 2;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

                THEN("Constant is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = constant;

                    if (cpuCopy.*reg & 0x80)
                    {
                        cpuCopy.N = 1;
                    }
                    else if (cpuCopy.*reg == 0x00)
                    {
                        cpuCopy.Z = 1;
                    }

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireState();
                }
            }
        }
    }

    void testLoadZeroPage(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("ZeroPage address is placed after instruction")
        {
            const u16 zeroPageAddr = 0x0037;
            const u8 zeroPageData = GENERATE(0x00, 0x42, 0xFF);
            memory[0xFFFC] = opCode;
            memory[0xFFFD] = zeroPageAddr;
            memory[zeroPageAddr] = zeroPageData;
            const s32 cyclesExpected = 3;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

                THEN("ZeroPage data is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = zeroPageData;

                    if (cpuCopy.*reg & 0x80)
                    {
                        cpuCopy.N = 1;
                    }
                    else if (cpuCopy.*reg == 0x00)
                    {
                        cpuCopy.Z = 1;
                    }

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireState();
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
            const u8 zeroPageData = GENERATE(0x00, 0x42, 0xFF);
            memory[0xFFFC] = opCode;
            memory[0xFFFD] = zeroPageAddr;
            memory[zeroPageAddrWithOffset] = zeroPageData;

            const s32 cyclesExpected = 4;
            const s32 PCIncrementsExpected = 2;

            takeSnapshot();

            WHEN(Cpu::OpCodeToString(opCode) + " is executed")
            {
                const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

                THEN("ZeroPage data is loaded into index register")
                {
                    cpuCopy.PC += PCIncrementsExpected;
                    cpuCopy.*reg = zeroPageData;

                    if (cpuCopy.*reg & 0x80)
                    {
                        cpuCopy.N = 1;
                    }
                    else if (cpuCopy.*reg == 0x00)
                    {
                        cpuCopy.Z = 1;
                    }

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireState();
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

                    if (cpuCopy.*reg & 0x80)
                    {
                        cpuCopy.N = 1;
                    }
                    else if (cpuCopy.*reg == 0x00)
                    {
                        cpuCopy.Z = 1;
                    }

                    REQUIRE(cyclesUsed == cyclesExpected);
                    requireState();
                }
            }
        }
    }
};

TEST_CASE_METHOD(CpuFixture, "CPU and memory reset")
{
    REQUIRE(cpu.PC == 0xFFFC); // Reset vector
    REQUIRE(cpu.SP == 0xFF);   // Top of the stack

    REQUIRE(cpu.A == 0);
    REQUIRE(cpu.X == 0);
    REQUIRE(cpu.Y == 0);

    // Check status register. Both the register and the individual bits
    REQUIRE(cpu.SR == 0);
    REQUIRE(cpu.C == 0);
    REQUIRE(cpu.Z == 0);
    REQUIRE(cpu.I == 0);
    REQUIRE(cpu.D == 0);
    REQUIRE(cpu.B == 0);
    REQUIRE(cpu.U == 0);
    REQUIRE(cpu.O == 0);
    REQUIRE(cpu.N == 0);

    // Check that memory is initialized to zeros
    const int sumOfAllAdresses = std::accumulate(std::begin(memory.data), std::end(memory.data), 0);
    REQUIRE(sumOfAllAdresses == 0);

    // Make sure that a reset is resetting everything correctly
    takeSnapshot();
    cpu.reset(memory);

    REQUIRE(cpu == cpuCopy);
    REQUIRE(memory == memoryCopy);
}

TEST_CASE_METHOD(CpuFixture, "No cycles")
{
    GIVEN("A reset CPU")
    {
        const s32 cyclesExpected = 0;

        takeSnapshot();

        WHEN("No cycles is executed")
        {
            const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

            THEN("Nothing happens")
            {
                REQUIRE(cyclesUsed == cyclesExpected);
                requireState();
            }
        }
    }
}

TEST_CASE_METHOD(CpuFixture, "Execute invalid instruction result in exception")
{
    REQUIRE_THROWS_AS(cpu.execute(1, memory), InvalidOpCode);
}

TEST_CASE_METHOD(CpuFixture, "NOP")
{
    GIVEN("Next instruction is NOP")
    {
        memory[0xFFFC] = Cpu::OP::NOP;

        const s32 PCIncrementsExpected = 1;
        const s32 cyclesExpected = 2;

        takeSnapshot();

        WHEN("NOP is executed")
        {
            const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

            THEN("Program counter is incremented")
            {
                cpuCopy.PC += PCIncrementsExpected;

                REQUIRE(cyclesUsed == cyclesExpected);
                requireState();
            }
        }
    }
}

TEST_CASE_METHOD(CpuFixture, "LDA")
{
    testLoadImmediate(Cpu::OP::LDA_IM, &Cpu::A);
    testLoadZeroPage(Cpu::OP::LDA_ZP, &Cpu::A);
    testLoadZeroPageOffset(Cpu::OP::LDA_ZPX, &Cpu::A, &Cpu::X);
    testLoadAbsolute(Cpu::OP::LDA_ABS, &Cpu::A);
}

TEST_CASE_METHOD(CpuFixture, "LDX")
{
    testLoadImmediate(Cpu::OP::LDX_IM, &Cpu::X);
    testLoadZeroPage(Cpu::OP::LDX_ZP, &Cpu::X);
    testLoadZeroPageOffset(Cpu::OP::LDX_ZPY, &Cpu::X, &Cpu::Y);
    testLoadAbsolute(Cpu::OP::LDX_ABS, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixture, "LDY")
{
    testLoadImmediate(Cpu::OP::LDY_IM, &Cpu::Y);
    testLoadZeroPage(Cpu::OP::LDY_ZP, &Cpu::Y);
    testLoadZeroPageOffset(Cpu::OP::LDY_ZPX, &Cpu::Y, &Cpu::X);
    testLoadAbsolute(Cpu::OP::LDY_ABS, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixture, "TXS")
{
    GIVEN("X is set and PC points to TXS")
    {
        cpu.X = 0x42;
        memory[0xFFFC] = Cpu::OP::TXS;

        const s32 PCIncrementsExpected = 1;
        const s32 cyclesExpected = 2;

        takeSnapshot();

        WHEN("TXS is executed")
        {
            const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

            THEN("Program counter is incremented and SP = X")
            {
                cpuCopy.PC += PCIncrementsExpected;
                cpuCopy.SP = cpuCopy.X;

                REQUIRE(cyclesUsed == cyclesExpected);
                requireState();
            }
        }
    }
}

} // namespace c6502
