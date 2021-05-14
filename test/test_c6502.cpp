#include "test_c6502.h"

namespace c6502
{
TEST_CASE_METHOD(CpuFixture, "CPU and memory reset")
{
    REQUIRE(cpu.PC == startAddr); // Reset vector
    REQUIRE(cpu.SP == 0xFF);      // Top of the stack

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
    cpu.reset(0x2000);
    cpuCopy.PC = 0x2000;

    REQUIRE(cpu == cpuCopy);
    REQUIRE(memory == memoryCopy);
}

#if 0
TEST_CASE_METHOD(CpuFixture, "No cycles")
{
    GIVEN("A reset system")
    {
        const s32 cyclesExpected = 0;

        takeSnapshot();

        WHEN("No cycles is executed")
        {
            const s32 cyclesUsed = cpu.execute();

            THEN("Nothing happens")
            {
                REQUIRE(cyclesUsed == cyclesExpected);
                requireState();
            }
        }
    }
}
#endif

TEST_CASE_METHOD(CpuFixture, "Execute invalid instruction result in exception")
{
    REQUIRE_THROWS_AS(cpu.execute(), InvalidOpCode);
}

TEST_CASE_METHOD(CpuFixture, "NOP")
{
    GIVEN("Next instruction is NOP")
    {
        memory[0x1000] = Cpu::OP::NOP;
        memory[0x1001] = Cpu::OP::HALT;

        const s32 PCIncrementsExpected = 1;
        const s32 cyclesExpected = 2;

        takeSnapshot();

        WHEN("NOP is executed")
        {
            const s32 cyclesUsed = cpu.execute();

            THEN("Program counter is incremented")
            {
                cpuCopy.PC += PCIncrementsExpected;

                REQUIRE(cyclesUsed == cyclesExpected);
                requireState();
            }
        }
    }
}

TEST_CASE_METHOD(CpuFixture, "TXS")
{
    GIVEN("X is set and PC points to TXS")
    {
        cpu.X = 0x42;
        memory[0x1000] = Cpu::OP::TXS;
        memory[0x1001] = Cpu::OP::HALT;

        const s32 PCIncrementsExpected = 1;
        const s32 cyclesExpected = 2;

        takeSnapshot();

        WHEN("TXS is executed")
        {
            const s32 cyclesUsed = cpu.execute();

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
