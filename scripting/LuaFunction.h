#pragma once
#include <typeindex>
#include <vector>
#include <sol/sol.hpp>
#include <lua.hpp>
#include "../deps/function2/function2.hpp"
#include "../core/CommonFunctions.h"
#include "../core/Dictionary.h"
#include "../core/Result.h"
#include "../core/FuncTraits.h"

namespace detail {

template <typename Sig>
struct extract_lua_fn_sig_hash;

template <typename Ret, typename...Args>
struct extract_lua_fn_sig_hash<Ret(Args...)>
{
	static inline size_t value = TypeIdHash<Ret, Args...>();
};

} // namespace detail

template <typename Sig>
inline size_t ExtractLuaFunctionSigHash()
{
	return detail::extract_lua_fn_sig_hash<Sig>::value;
}

template <typename Ret>
concept ValidLuaFnResult = (std::same_as<Ret, void> || std::is_default_constructible_v<Ret>);

namespace detail {

template <typename Sig>
struct valid_lua_fn_sig;

template <typename Ret, typename...Args>
struct valid_lua_fn_sig<Ret(Args...)> : std::bool_constant<ValidLuaFnResult<Ret>> {};

} // namespace detail

template <typename Sig>
concept ValidLuaFnSig = detail::valid_lua_fn_sig<Sig>::value;

namespace detail {

template <ValidLuaFnResult Ret>
Ret default_ret()
{
	if constexpr (std::same_as<Ret, void>) { return; }
	else { return Ret{}; }
}

template <typename Sig>
struct convert_lua_to_fu2_fn;

template <typename Ret, typename...Args> requires ValidLuaFnResult<Ret>
struct convert_lua_to_fu2_fn<Ret(Args...)>
{
	static fu2::unique_function<Ret(Args...)> call(sol::function&& solFn) 
	{
		return [f = std::move(solFn)](Args...args) -> Ret {
			if (!f.valid())
			{
				LOG_ERROR("Sol function was invalid");

				return default_ret<Ret>();
			}

			sol::protected_function_result result = f.call(args...);
			if (!result.valid())
			{
				sol::error err = result;
				LOG_ERROR(err.what());

				return default_ret<Ret>();
			}

			if constexpr (std::same_as<Ret, void>)
			{
				return;
			}
			else
			{
				sol::object obj = result;
				if (obj.is<raw_type_t<Ret>>())
				{
					return obj.as<Ret>();
				}
				else
				{
					LOG_ERROR("Sol function did not return expected type");

					return default_ret<Ret>();
				}
			}
		};
	}
};

} // namespace detail

template <typename Sig>
inline fu2::unique_function<Sig> ConvertLuaToFu2Function(sol::function solFn)
{
	return detail::convert_lua_to_fu2_fn<Sig>::call(std::move(solFn));
}

class LuaFunctionTable
{
public:
	explicit LuaFunctionTable(sol::state& state) : luaState_(state) {}

	template <ValidLuaFnSig Sig>
	fu2::unique_function<Sig> GetFunction(std::string_view nm)
	{
		auto it = fnSigHashes_.find(nm);
		if (it == fnSigHashes_.end())
		{
			return {};
		}
		if (it->second != ExtractLuaFunctionSigHash<Sig>())
		{
			return {};
		}
		if (!luaState_[nm].valid())
		{
			return {};
		}

		return ConvertLuaToFu2Function<Sig>(luaState_[nm]);
	}

	template <ValidLuaFnSig Sig>
	fu2::unique_function<Sig> RegisterFunction(std::string_view nm)
	{
		if (fnSigHashes_.contains(nm))
		{
			return {};
		}
		if (!(luaState_[nm].valid() && luaState_[nm].get_type() == sol::type::function))
		{
			return {};
		}

		fnSigHashes_[nm] = ExtractLuaFunctionSigHash<Sig>();

		return ConvertLuaToFu2Function<Sig>(luaState_[nm]);
	}

	template <ValidLuaFnSig Sig>
	bool HasFunction(std::string_view nm) const
	{
		auto it = fnSigHashes_.find(nm);

		return it != fnSigHashes_.end() && it->second == ExtractLuaFunctionSigHash<Sig>();
	}

private:
	sol::state_view luaState_;
	UnorderedDictionary<size_t> fnSigHashes_;
};

//template <typename Sig>
//class LuaFunction;
//
//template <typename Ret, typename...Args>
//class LuaFunction<Ret(Args...)>
//{
//public:
//	LuaFunction() = default;
//	explicit LuaFunction(sol::function&& fn) : fn_(std::move(fn)) {}
//
//	bool operator!() const { return !fn_.valid(); }
//
//	template <typename...Ts> requires (std::convertible_to<Ts, Args> && ...)
//		Ret operator()(Ts&&...ts)
//	{
//
//	}
//
//private:
//	sol::function fn_;
//};

//namespace detail {
//	template <typename T>
//	struct is_lua_function;
//	template <typename Sig>
//	struct is_lua_function<LuaFunction<Sig>> : std::true_type {};
//}
//
//template <typename T>
//inline constexpr bool is_lua_function_v = detail::is_lua_function<T>::value;