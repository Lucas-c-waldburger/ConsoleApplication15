#pragma once
#include "../deps/nlohmann/json.hpp"
#include "../core/Result.h"
#include "../core/commonObjects.h"
#include <limits>


template <typename T>
inline Result<T> GetJsonNativeValue(const nlohmann::json& j, std::string_view keyName)
{
	auto makeErrorMsg = [&] {
		return std::format("Json value for key '{}' was not safely convertible from "
			"json type '{}' to requested type '{}'",
			keyName, j.type_name(), typeid(T).name());
	};

	if (!j.contains(keyName))
	{
		return MAKE_ERROR_FMT("Json did not contain key '{}'", keyName);
	}

	const auto& val = j.at(keyName);

	if (val.is_array() || val.is_object())
	{
		return MAKE_ERROR(makeErrorMsg());
	}
	if constexpr (std::same_as<T, bool>)
	{
		if (!val.is_boolean())
		{
			return MAKE_ERROR(makeErrorMsg());
		}
		return val.get<bool>();
	}
	else if constexpr (std::same_as<T, std::string>)
	{
		if (!val.is_string())
		{
			return MAKE_ERROR(makeErrorMsg());
		}
		return val.get<std::string>();
	}
	else if constexpr (std::is_arithmetic_v<T>)
	{
		if (!val.is_number())
		{
			return MAKE_ERROR(makeErrorMsg());
		}

		if constexpr (std::is_integral_v<T>)
		{
			if constexpr (std::is_unsigned_v<T>)
			{
				if (!val.is_number_unsigned())
				{
					return MAKE_ERROR(makeErrorMsg());
				}

				uint64_t temp = val.get<uint64_t>();
				if (temp > static_cast<uint64_t>(std::numeric_limits<T>::max()))
				{
					return MAKE_ERROR("Unsigned value in Json exceeds "
						" size of requested type");
				}

				return static_cast<T>(temp);
			}
			else // signed
			{
				if (val.is_number_unsigned())
				{
					return MAKE_ERROR(makeErrorMsg());
				}

				int64_t temp = val.get<int64_t>();
				if (temp > static_cast<int64_t>(std::numeric_limits<T>::max()) ||
					temp < static_cast<int64_t>(std::numeric_limits<T>::min()))
				{
					return MAKE_ERROR("Signed value in Json exceeds "
						" size of requested type");
				}

				return static_cast<T>(temp);
			}
		}
		else // floating point
		{
			if (!val.is_number_float())
			{
				return MAKE_ERROR(makeErrorMsg());
			}

			double temp = val.get<double>();
			if (temp > static_cast<double>(std::numeric_limits<T>::max()) ||
				temp < static_cast<double>(std::numeric_limits<T>::min()))
			{
				return MAKE_ERROR("Signed floating point value in Json exceeds "
					" size of requested type");
			}

			return static_cast<T>(temp);
		}		
	}
	else
	{
		static_assert(false);
		return T{};
	}
}

template <typename T>
inline Result<Void> GetToJsonUserObject(const nlohmann::json& j, T& output, 
									    std::string_view keyName)
{
	if (!j.contains(keyName))
	{
		return MAKE_ERROR_FMT("Json did not contain key '{}'", keyName);
	}

	const auto& obj = j.at(keyName);
	if (!obj.is_object())
	{
		return MAKE_ERROR_FMT("Json value at key '{}' is not of object type", keyName);
	}

	obj.get_to(output);

	return Void{};
}