#pragma once
#include <sol/sol.hpp>
#include "../core/Result.h"
#include "../core/TypeUtils.h"

template <typename Sig>
struct TypedLuaFunction;

template<typename Ret, typename...Args> requires (std::same_as<Ret, std::remove_cvref_t<Ret>>)
struct TypedLuaFunction<Ret(Args...)> 
{
    sol::function func;

    template <typename...CallArgs> requires std::same_as<TypeList<CallArgs...>, TypeList<Args...>>
    Result<Ret> operator()(CallArgs&&...args) const
    {
        if (!func.valid())
        {
            return MAKE_ERROR("sol function was invalid");
        }

        sol::protected_function_result result = func(std::forward<CallArgs>(args)...);
        if (!result.valid())
        {
            sol::error err = result;
            return MAKE_ERROR(err.what());
        }

        sol::object retObj = result;
        if (retObj.is<Ret>())
        {
            return retObj.as<Ret>();
        }
        else
        {
            return MAKE_ERROR("Lua function did not return expected type");
        }
    }
};
