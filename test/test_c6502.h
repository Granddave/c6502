#include "catch2/catch.hpp"

#include "c6502/c6502.h"

namespace c6502
{
class CpuFixture
{
public:
    Cpu cpu;
    Memory memory;

    Cpu cpuCopy;
    Memory memoryCopy;

    /// Initialize the memory with an arbitrary start address
    static constexpr u16 startAddr = 0x1000;

    CpuFixture()
    {
        cpu.reset(startAddr);
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
};

} // namespace c6502
