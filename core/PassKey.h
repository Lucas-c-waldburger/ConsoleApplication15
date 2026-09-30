#pragma once

template <typename T>
class PassKey;

template <typename T>
class PassKeyAccess
{
protected:
	static constexpr PassKey<T> GetPassKey();
};

template <typename T>
class PassKey
{
	friend class PassKeyAccess<T>;
	
	constexpr PassKey() = default;

	static constexpr PassKey Get() { return {}; }
};

template <typename T>
constexpr PassKey<T> PassKeyAccess<T>::GetPassKey()
{
	return PassKey<T>::Get();
}