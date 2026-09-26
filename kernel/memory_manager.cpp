#include "memory_manager.hpp"

BitmapMemoryManager::BitmapMemoryManager() : alloc_map_{}, range_begin_{FrameID{0}}, range_end_{FrameID{kFrameCount}} {}

// 連続した領域を割り当てる
WithError<FrameID> BitmapMemoryManager::Allocate(size_t num_frames)
{
    size_t start_frame_id = range_begin_.ID();

    while (true)
    {
        size_t i = 0;
        for (; i < num_frames; i++)
        {

            if (start_frame_id + i >= range_end_.ID())
            {
                return {kNullFrame, MAKE_ERROR(Error::kNoEnoughMemory)};
            }
            if (GetBit(FrameID{start_frame_id + i}))
            {
                break;
            }
        }

        if (i == num_frames)
        {
            MarkAllocated(FrameID{start_frame_id}, num_frames);
            return {
                FrameID{start_frame_id},
                MAKE_ERROR(Error::kSuccess)};
        }

        start_frame_id += i + 1;
    }
}

Error BitmapMemoryManager::Free(FrameID start_frame, size_t num_frames)
{
    for (size_t i = 0; i < num_frames; i++)
    {
        SetBit(FrameID{start_frame.ID() + i}, false);
    }

    return MAKE_ERROR(Error::kSuccess);
}

void BitmapMemoryManager::MarkAllocated(FrameID start_frame, size_t num_frames)
{
    for (size_t i = 0; i < num_frames; i++)
    {
        SetBit(FrameID{start_frame.ID() + i}, true);
    }
}

void BitmapMemoryManager::SetMemoryRange(FrameID range_begin, FrameID range_end)
{
    range_begin_ = range_begin;
    range_end_ = range_end;
}

bool BitmapMemoryManager::GetBit(FrameID frame)
{
    const auto map_idx = frame.ID() / kBitsPerLine;
    const auto bit_idx = frame.ID() % kBitsPerLine;

    return (alloc_map_[map_idx] & (static_cast<MapLineType>(1) << bit_idx)) != 0;
}

void BitmapMemoryManager::SetBit(FrameID frame, bool allocated)
{
    const auto map_idx = frame.ID() / kBitsPerLine;
    const auto bit_idx = frame.ID() % kBitsPerLine;

    if (allocated)
    {
        alloc_map_[map_idx] |= static_cast<MapLineType>(1) << map_idx;
    }
    else
    {
        alloc_map_[map_idx] &= ~static_cast<MapLineType>(1) << bit_idx;
    }
}