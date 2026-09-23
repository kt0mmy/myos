#pragma once

enum class DescriptorType
{
    // IDT
    kInterruptGate = 14,

    // GDT
    kReadWrite = 2,
    kExecuteRead = 10,
};