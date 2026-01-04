#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <bit>
#include "../components/UserComponents.h"
#include "../ecs/ComponentManager.h"

using UserComponentTypeId = uint32_t;

inline UserComponentTypeId NextUserComponentTypeId()
{
	static UserComponentTypeId next = 0;
	return next++;
}

template <typename T>
inline UserComponentTypeId GetUserComponentTypeId()
{
	static UserComponentTypeId id = NextUserComponentTypeId();
	return id;
}

//template <typename Fn, typename T>
//concept SomeUnaryTypePred = requires(Fn fn) {
//	{ fn.template operator()<T>() } -> std::same_as<bool>;
//};

static constexpr bool kStopPackExpansion = false;
static constexpr bool kContinuePackExpansion = true;

template <typename TList>
struct ForEachUserComponentTypeImpl;

template <typename...Ts>
struct ForEachUserComponentTypeImpl<TypeList<Ts...>>
{
	template <typename Fn>
	static bool Apply(Fn&& fn)
	{
		return !(fn.template operator()<Ts>() && ...);
	}
};

template <typename Fn>
inline bool ForEachUserComponentType(Fn&& fn)
{
	bool result = false;
	return ForEachUserComponentTypeImpl<UserComponentTypeList>::template Apply(
		[&fn]<typename UserCmpType> -> bool {
			return fn.template operator()<UserCmpType>();
		});
}

template <typename Fn>
inline bool ForMatchingUserComponentType(ComponentSignature cmpBit, Fn&& fn)
{
	return ForEachUserComponentTypeImpl<UserComponentTypeList>::template Apply(
		[&]<typename UserCmpType> -> bool {
			if (UserCmpType::componentBit == cmpBit)
			{
				fn.template operator()<UserCmpType>();

				return kStopPackExpansion;
			}
			return kContinuePackExpansion;
		});
}


class UserComponentBridge
{
public:
	size_t GetAvailableComponentCount() const
	{
		return UserComponentTypeList::size - std::popcount(claimedUserComponentBits_);
	}

	template <typename T>
	bool RegisterComponentData()
	{
		const auto typeId = GetUserComponentTypeId<T>();
		if (componentBitForDataType_.contains(typeId))
		{
			return false;
		}
		if (GetAvailableComponentCount() <= 0)
		{
			return false;
		}

		const bool success = ForEachUserComponentType(
			[&]<typename UserCmpType> -> bool {
				if ((claimedUserComponentBits_ & UserCmpType::componentBit) == 0)
				{
					componentBitForDataType_[typeId] = UserCmpType::componentBit;
					claimedUserComponentBits_ |= UserCmpType::componentBit;

					return kStopPackExpansion;
				}
				return kContinuePackExpansion;
			});

		assert(success);

		return true;
	}

	template <typename T, typename...Args>
		requires std::constructible_from<T, Args...>
	T& AddComponentData(Entity_t entity, ComponentManager& cmpManager,
						 Args&&...args)
	{
		const auto typeId = GetUserComponentTypeId<T>();
		if (!componentBitForDataType_.contains(typeId))
		{
			const bool registered = RegisterComponentData<T>();
			assert(registered);
		}

		const ComponentSignature userCmpBit = componentBitForDataType_.at(typeId);
		T* temp = nullptr;

		bool success = ForMatchingUserComponentType(userCmpBit,
			[&]<typename UserCmpType> {
				auto& userCmp = cmpManager.AddComponent<UserCmpType>(entity);
				temp = &userCmp.data.Emplace<T>(std::forward<Args>(args)...);
			});

		assert(success);

		return *temp;
	}

	template <typename T>
	T& GetComponentData(Entity_t entity, ComponentManager& cmpManager)
	{
		const auto typeId = GetUserComponentTypeId<T>();
		assert(componentBitForDataType_.contains(typeId));

		const ComponentSignature userCmpBit = componentBitForDataType_.at(typeId);
		T* temp = nullptr;

		bool success = ForMatchingUserComponentType(userCmpBit,
			[&]<typename UserCmpType> {
				assert(cmpManager.HasComponent<UserCmpType>(entity));
				auto& userCmp = cmpManager.GetComponent<UserCmpType>(entity);

				assert(userCmp.data.HasValue());
				temp = &userCmp.data.Get<T>();
			});

		assert(success);
		assert(temp);

		return *temp;
	}

	template <typename T>
	const T& GetComponentData(Entity_t entity, const ComponentManager& cmpManager) const
	{
		const auto typeId = GetUserComponentTypeId<T>();
		assert(componentBitForDataType_.contains(typeId));

		const ComponentSignature userCmpBit = componentBitForDataType_.at(typeId);
		const T* temp = nullptr;

		bool success = ForMatchingUserComponentType(userCmpBit,
			[&]<typename UserCmpType> {
				assert(cmpManager.HasComponent<UserCmpType>(entity));
				const auto& userCmp = cmpManager.GetComponent<UserCmpType>(entity);

				assert(userCmp.data.HasValue());
				temp = &userCmp.data.Get<T>();
			});

		assert(success);
		assert(temp);

		return *temp;
	}

	template <typename T>
	void RemoveComponentData(Entity_t entity, ComponentManager& cmpManager)
	{
		const auto typeId = GetUserComponentTypeId<T>();
		if (!componentBitForDataType_.contains(typeId))
		{
			return;
		}

		const ComponentSignature userCmpBit = componentBitForDataType_.at(typeId);

		ForMatchingUserComponentType(userCmpBit, [&]<typename UserCmpType> {
			cmpManager.RemoveComponent<UserCmpType>(entity);
		});
	}

	template <typename T>
	bool HasComponentData(Entity_t entity, const ComponentManager& cmpManager) const
	{
		const auto typeId = GetUserComponentTypeId<T>();
		if (!componentBitForDataType_.contains(typeId))
		{
			return false;
		}

		const ComponentSignature userCmpBit = componentBitForDataType_.at(typeId);
		bool result = false;

		ForMatchingUserComponentType(userCmpBit, [&]<typename UserCmpType> {
			result = cmpManager.HasComponent<UserCmpType>(entity);
		});

		return result;
	}
	 
	template <typename T>
	bool IsComponentDataRegistered() const
	{
		return componentBitForDataType_.contains(GetUserComponentTypeId<T>());
	}

	template <typename T>
	ComponentSignature GetComponentDataSignature() const
	{
		auto it = componentBitForDataType_.find(GetUserComponentTypeId<T>());
		
		return (it != componentBitForDataType_.end()) ? it->second : 0;
	}

private:
	std::unordered_map<UserComponentTypeId, ComponentSignature> componentBitForDataType_;
	ComponentSignature claimedUserComponentBits_ = 0;
};

