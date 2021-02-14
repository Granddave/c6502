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
    void requireState(const s32 cyclesUsed, const s32 cyclesExpected)
    {
        REQUIRE(cyclesUsed == cyclesExpected);
        REQUIRE(memory == memoryCopy);
        REQUIRE(cpu == cpuCopy);
    }

    void testLoadImmediate(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("Constant is placed after instruction")
        {
            const u8 constant = 0x42;
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

                    requireState(cyclesUsed, cyclesExpected);
                }
            }
        }
    }

    void testLoadZeroPage(const Cpu::OP opCode, u8 Cpu::*reg)
    {
        GIVEN("ZeroPage address is placed after instruction")
        {
            const u16 zeroPageAddr = 0x0037;
            const u8 zeroPageData = 0x42;
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

                    requireState(cyclesUsed, cyclesExpected);
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

TEST_CASE_METHOD(CpuFixture, "NOP")
{
    GIVEN("Next instruction is NOP")
    {
        memory[0xFFFC] = Cpu::OP::NOP;
        const s32 cyclesExpected = 2;

        takeSnapshot();

        WHEN("NOP is executed")
        {
            const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

            THEN("Program counter is incremented")
            {
                cpuCopy.PC++;

                requireState(cyclesUsed, cyclesExpected);
            }
        }
    }
}

TEST_CASE_METHOD(CpuFixture, "LDA")
{
    testLoadImmediate(Cpu::OP::LDA_IM, &Cpu::A);
    testLoadZeroPage(Cpu::OP::LDA_ZP, &Cpu::A);
}

TEST_CASE_METHOD(CpuFixture, "LDX")
{
    testLoadImmediate(Cpu::OP::LDX_IM, &Cpu::X);
    testLoadZeroPage(Cpu::OP::LDX_ZP, &Cpu::X);
}

TEST_CASE_METHOD(CpuFixture, "LDY")
{
    testLoadImmediate(Cpu::OP::LDY_IM, &Cpu::Y);
    testLoadZeroPage(Cpu::OP::LDY_ZP, &Cpu::Y);
}

TEST_CASE_METHOD(CpuFixture, "TXS")
{
    GIVEN("X is set and PC points to TXS")
    {
        cpu.X = 0x42;
        memory[0xFFFC] = Cpu::OP::TXS;
        const s32 cyclesExpected = 2;

        takeSnapshot();

        WHEN("TXS is executed")
        {
            const s32 cyclesUsed = cpu.execute(cyclesExpected, memory);

            THEN("Program counter is incremented and SP = X")
            {
                cpuCopy.PC++;
                cpuCopy.SP = cpuCopy.X;

                requireState(cyclesUsed, cyclesExpected);
            }
        }
    }
}

} // namespace c6502
