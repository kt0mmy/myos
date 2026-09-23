#pragma once
#include <array>

// Error::kSuccess は暗黙的に Error{Error::kSuccess}
// Error(Code code)が1つの引数だけなので、Error::Code→Errorという返還になる
// explicit Error(Code code)とかくとこの挙動を抑えられる
class Error
{
public:
    enum Code
    {
        kSuccess,
        kFull,
        kEmpty,
        kIndexOutOfRange,
        kNotImplemented,
        kInvalidDescriptor,
        kInvalidPhase,
        kNoWaiter,
        kNoEnoughMemory,
        kInvalidSlotID,
        kUnknownXHCISpeedID,
        kTransferFailed,
        kNoCorrespondingSetupStage,
        kTransferRingNotSet,
        kInvalidEndpointNumber,
        kAlreadyAllocated,
        kNoPCIMSI,
        kLastOfCode,
    };

    Error(Code code, const char *file, int line) : code_{code}, line_{line}, file_{file} {}

    operator bool() const
    {
        return this->code_ != kSuccess;
    }

    const char *Name() const
    {
        return code_names_[code_];
    }

    const char *File() const
    {
        return this->file_;
    }

    int Line() const
    {
        return this->line_;
    }

private:
    static constexpr std::array code_names_ = {
        "kSuccess",
        "kFull",
        "kEmpty",
        "kIndexOutOfRange",
        "kNotImplemented",
        "kInvalidDescriptor",
        "kInvalidPhase",
        "kNoWaiter",
        "kNoEnoughMemory",
        "kInvalidSlotID",
        "kUnknownXHCISpeedID",
        "kTransferFailed",
        "kNoCorrespondingSetupStage",
        "kTransferRingNotSet",
        "kInvalidEndpointNumber",
        "kAlreadyAllocated",
        "kNoPCIMSI",
    };

    Code code_;
    int line_;
    const char *file_;
};

#define MAKE_ERROR(code) Error((code), __FILE__, __LINE__)
template <class T>
struct WithError
{
    T value;
    Error error;
};
