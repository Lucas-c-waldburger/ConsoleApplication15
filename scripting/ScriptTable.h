#pragma once
#include "ScriptSignature.h"
#include "../core/Dictionary.h"
#include <sol/sol.hpp>

class ScriptTableView;

class ScriptTable
{
public:
    struct CallableWrapper
    {
    public:
        CallableWrapper() = default;
        CallableWrapper(const ScriptSignature& sig, sol::function&& fn)
            : signature_(sig), fn_(std::move(fn)) {}
		~CallableWrapper() = default;
		CallableWrapper(const CallableWrapper&) = delete;
		CallableWrapper& operator=(const CallableWrapper&) = delete;
		CallableWrapper(CallableWrapper&&) noexcept = default;
		CallableWrapper& operator=(CallableWrapper&&) noexcept = default;

        bool IsValid() const
        {
            return signature_.IsValid() && fn_.valid();
        }

        template <HasFuncTraits Sig>
        bool Matches() const
        {
            return signature_.Matches<Sig>();
        }

        template <typename...Args>
        bool MatchesArguments() const
        {
            return signature_.MatchesArguments<Args...>();
        }

        template <typename...Args>
        decltype(auto) operator()(Args&&...args)
        {
            return fn_(std::forward<Args>(args)...);
        }

    private:
        ScriptSignature signature_;
        sol::function fn_;
    };

    using TableId = uint32_t;

    ScriptTable() = default;
	~ScriptTable() = default;
	ScriptTable(const ScriptTable&) = delete;
	ScriptTable& operator=(const ScriptTable&) = delete;
	ScriptTable(ScriptTable&&) noexcept = default;
	ScriptTable& operator=(ScriptTable&&) noexcept = default;

    template <HasFuncTraits Sig>
    bool RegisterFunction(std::string_view fnName)
    {
        if (registeredSignatures_.contains(fnName))
        {
            return false;
        }

        if (!ValidateFunction(fnName))
        {
            return false;
        }

        return registeredSignatures_.emplace(fnName, ScriptSignature::Create<Sig>()).second;
    }

    template <HasFuncTraits Sig>
    bool Contains(std::string_view fnName) const
    {
        auto it = registeredSignatures_.find(fnName);

        return it != registeredSignatures_.end() && it->second.Matches<Sig>();
    }

    CallableWrapper operator[](std::string_view fnName) const
    {
        auto it = registeredSignatures_.find(fnName);
        if (it == registeredSignatures_.end())
        {
            return {};
        }

        return { it->second, table_[fnName] };
    }

    ScriptTableView GetView() const;

    TableId GetTableId() const noexcept { return id_; }

    static ScriptTable Create(sol::table&& table)
    {
        ScriptTable scriptTable{};

        scriptTable.table_ = std::move(table);
        scriptTable.id_ = ++tableIdCounter_;

        return scriptTable;
    }

private:
    bool ValidateFunction(std::string_view fnName)
    {
        const auto& fn = table_[fnName];

        return fn.valid() && fn.get_type() == sol::type::function;
    }

	static inline TableId tableIdCounter_ = 0;

    sol::table table_;
	TableId id_ = std::numeric_limits<TableId>::max();
    UnorderedDictionary<ScriptSignature> registeredSignatures_;
};

class ScriptTableView
{
public:
    ScriptTableView() = default;
    explicit ScriptTableView(const ScriptTable& tbl) : scriptTable_(&tbl) {}

    ScriptTable::CallableWrapper operator[](std::string_view fnName) const
    {
        if (!scriptTable_)
        {
            return {};
        }
        return (*scriptTable_)[fnName];
    }

    template <HasFuncTraits Sig>
    bool Contains(std::string_view fnName) const
    {
        if (!scriptTable_)
        {
            return false;
        }
        return scriptTable_->Contains<Sig>(fnName);
    }

    bool IsValid() const noexcept
    {
        return scriptTable_ && scriptTable_->GetTableId() != 
               std::numeric_limits<ScriptTable::TableId>::max();
    }

    ScriptTable::TableId GetTableId() const noexcept
    {
        if (!scriptTable_)
        {
            return std::numeric_limits<ScriptTable::TableId>::max();
        }
        return scriptTable_->GetTableId();
	}

    constexpr bool operator==(const ScriptTableView& rhs) const noexcept
    {
        return scriptTable_ == rhs.scriptTable_;
	}

private:
    const ScriptTable* scriptTable_ = nullptr;
};