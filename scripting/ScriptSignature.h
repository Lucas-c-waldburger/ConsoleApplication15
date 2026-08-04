#pragma once
#include "../core/FuncTraits.h"
#include <cassert>
#include <limits>

using ScriptArgumentId = uint8_t;

inline ScriptArgumentId NextScriptArgumentId()
{
    static ScriptArgumentId next = 1;
    assert(next < std::numeric_limits<ScriptArgumentId>::max());
    return next++;
}

template <typename T>
inline ScriptArgumentId GetScriptArgumentId()
{
    static ScriptArgumentId id = NextScriptArgumentId();
    return id;
}

inline constexpr ScriptArgumentId kInvalidScriptArgumentId = 0;

class ScriptSignature
{
public:
    static constexpr uint8_t kMaxArguments = 6;

    constexpr ScriptSignature() = default;

    constexpr ScriptArgumentId ReturnType() const
    {
        return static_cast<ScriptArgumentId>((bits_ >> 56) & 0xff);
    }

    constexpr uint8_t ArgumentCount() const
    {
        return static_cast<uint8_t>((bits_ >> 48) & 0xff);
    }

    constexpr ScriptArgumentId Argument(size_t index) const
    {
        if (index >= ArgumentCount())
        {
            return kInvalidScriptArgumentId;
        }

        return static_cast<ScriptArgumentId>((bits_ >> (index * 8)) & 0xff);
    }

    constexpr bool ContainsArgument(ScriptArgumentId id) const
    {
        for (uint8_t i = 0; i < ArgumentCount(); ++i)
        {
            if (Argument(i) == id)
            {
                return true;
            }
        }

        return false;
    }

    template <typename T>
    bool ContainsArgument() const
    {
        return ContainsArgument(GetScriptArgumentId<T>());
    }

    constexpr int IndexOfArgument(ScriptArgumentId id) const
    {
        for (uint8_t i = 0; i < ArgumentCount(); ++i)
        {
            if (Argument(i) == id)
            {
                return i;
            }
        }

        return -1;
    }

    template <typename T>
    int IndexOfArgument() const
    {
        return IndexOfArgument(GetScriptArgumentId<T>());
    }

    constexpr uint64_t Bits() const
    {
        return bits_;
    }

    constexpr bool IsValid() const
    {
        return bits_ != 0;
    }

    template <HasFuncTraits Sig>
    static ScriptSignature Create()
    {
        ScriptSignature signature{};

        signature.SetReturnType<typename func_traits<Sig>::return_type>();

        static constexpr size_t argCount = func_traits<Sig>::argCount;

        static_assert(argCount <= ScriptSignature::kMaxArguments,
            "Function has too many arguments for script signature");

        AddArgumentsToSignature<Sig, argCount>(signature);

        return signature;
    }

    constexpr bool operator==(const ScriptSignature& rhs) const
    {
        return bits_ == rhs.bits_;
	}

    template <HasFuncTraits Sig>
    bool Matches() const
    {
        return *this == Create<Sig>();
    }

    template <typename...Args>
    bool MatchesArguments() const
    {
        ScriptSignature signature{};

        signature.SetReturnType(ReturnType());
        (signature.AddArgument<Args>(), ...);

        return *this == signature;
    }

private:
    template <typename T>
    void SetReturnType()
    {
        SetReturnType(GetScriptArgumentId<T>());
    }

    constexpr void SetReturnType(ScriptArgumentId id)
    {
        bits_ &= ~(static_cast<uint64_t>(0xff) << 56);
        bits_ |= static_cast<uint64_t>(id) << 56;
    }

    constexpr void AddArgument(ScriptArgumentId id)
    {
        auto count = ArgumentCount();
        if (count >= kMaxArguments)
        {
            return;
        }

        bits_ |= static_cast<uint64_t>(id) << (count * 8);

        bits_ &= ~(static_cast<uint64_t>(0xff) << 48);
        bits_ |= static_cast<uint64_t>(count + 1) << 48;
    }

    template <typename T>
    void AddArgument()
    {
        AddArgument(GetScriptArgumentId<T>());
    }

    template <HasFuncTraits Sig, size_t I>
    static void AddArgumentsToSignature(ScriptSignature& signature)
    {
        if constexpr (I > 0)
        {
            using ArgType = typename func_traits<Sig>::template arg_at<I - 1>;

            AddArgumentsToSignature<Sig, I - 1>(signature);
            signature.AddArgument<ArgType>();
        }
    }

    uint64_t bits_ = 0;
};