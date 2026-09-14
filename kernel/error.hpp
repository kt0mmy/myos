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
        kLastOfCode,
    };

    Error(Code code) : code_{code} {}

    operator bool() const
    {
        return this->code_ != kSuccess;
    }

    const char *Name() const
    {
        return code_names_[code_];
    }

private:
    static constexpr std::array<const char *, 3> code_names_ = {
        "kSuccess",
        "kFull",
        "kEmpty"};

    Code code_;
};