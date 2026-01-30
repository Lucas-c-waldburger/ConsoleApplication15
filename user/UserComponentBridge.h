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

template <typename UserCmpList>
struct UserComponentDispatchTableImpl;

template <template <typename...> class UserCmpList, typename...Ts>
class UserComponentDispatchTableImpl<UserCmpList<Ts...>>
{
	UserComponentDispatchTableImpl() = default;

	template <typename T>
	static InlineStorage<kUserComponentStorageSize>& 
	AddUserComponentStorageImpl(Entity_t entity, ComponentManager& cmpManager)
	{
		return cmpManager.AddComponent<T>(entity).data;
	}

	template <typename T>
	static InlineStorage<kUserComponentStorageSize>&
	GetUserComponentStorageImpl(Entity_t entity, ComponentManager& cmpManager)
	{
		assert(cmpManager.HasComponent<T>(entity));

		return cmpManager.GetComponent<T>(entity).data;
	}

	using AddGetUserComponentStorageSig = InlineStorage<kUserComponentStorageSize>&(*)
										  (Entity_t, ComponentManager&);

	template <typename T>
	static const InlineStorage<kUserComponentStorageSize>&
	GetConstUserComponentStorageImpl(Entity_t entity, const ComponentManager& cmpManager)
	{
		assert(cmpManager.HasComponent<T>(entity));

		return cmpManager.GetComponent<T>(entity).data;
	}

	using GetConstUserComponentStorageSig = 
		const InlineStorage<kUserComponentStorageSize>&(*)
		(Entity_t, const ComponentManager&);

	template <typename T>
	static void RemoveUserComponentImpl(Entity_t entity, ComponentManager& cmpManager)
	{
		cmpManager.RemoveComponent<T>(entity);
	}

	using RemoveUserComponentSig = void(*)(Entity_t, ComponentManager&);

	template <typename T>
	static bool HasUserComponentImpl(Entity_t entity, const ComponentManager& cmpManager)
	{
		return cmpManager.HasComponent<T>(entity);
	}

	using HasUserComponentSig = bool(*)(Entity_t, const ComponentManager&);

	template <typename T>
	static ComponentSignature GetUserComponentBitImpl()
	{
		return T::componentBit;
	}

	using GetUserComponentBitSig = ComponentSignature(*)(void);

public:
	static constexpr AddGetUserComponentStorageSig kAddUserComponentStorage[] = { 
		&AddUserComponentStorageImpl<Ts>... 
	};

	static constexpr AddGetUserComponentStorageSig kGetUserComponentStorage[] = {
		&GetUserComponentStorageImpl<Ts>...
	};

	static constexpr GetConstUserComponentStorageSig kGetConstUserComponentStorage[] = {
		&GetConstUserComponentStorageImpl<Ts>...
	};

	static constexpr RemoveUserComponentSig kRemoveUserComponent[] = {
		&RemoveUserComponentImpl<Ts>...
	};

	static constexpr HasUserComponentSig kHasUserComponent[] = {
		&HasUserComponentImpl<Ts>...
	};

	static constexpr GetUserComponentBitSig kGetUserComponentBit[] = {
		&GetUserComponentBitImpl<Ts>...
	};
};

using UserComponentDispatchTable = UserComponentDispatchTableImpl<UserComponentTypeList>;

class UserComponentBridge
{
public:
	size_t GetAvailableComponentCount() const
	{
		assert(nextFreeComponentIndex_ <= UserComponentTypeList::size);

		return (nextFreeComponentIndex_ >= UserComponentTypeList::size)
			? 0
			: UserComponentTypeList::size - nextFreeComponentIndex_;
	}

	template <typename T>
	bool RegisterComponentData()
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
		if (IsComponentDataRegisteredInternal(typeId))
		{
			return true;
		}

		return RegisterComponentDataInternal(typeId);
	}

	template <typename T, typename...Args>
		requires std::constructible_from<T, Args...>
	T& AddComponentData(Entity_t entity, ComponentManager& cmpManager, Args&&...args)
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
		
		if (!IsComponentDataRegisteredInternal(typeId))
		{
			const bool registered = RegisterComponentDataInternal(typeId);
			assert(registered);
		}

		const size_t idx = userComponentListIndexForDataType_[typeId];
		assert(idx < UserComponentTypeList::size);

		InlineStorage<kUserComponentStorageSize>& newCmpStorage =
			std::invoke(UserComponentDispatchTable::kAddUserComponentStorage[idx],
						entity, cmpManager);

		return newCmpStorage.Emplace<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	T& GetComponentData(Entity_t entity, ComponentManager& cmpManager)
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
		assert(IsComponentDataRegisteredInternal(typeId));

		const size_t idx = userComponentListIndexForDataType_[typeId];
		assert(idx < UserComponentTypeList::size);

		InlineStorage<kUserComponentStorageSize>& cmpStorage =
			std::invoke(UserComponentDispatchTable::kGetUserComponentStorage[idx],
						entity, cmpManager);

		return cmpStorage.Get<T>();
	}

	template <typename T>
	const T& GetComponentData(Entity_t entity, const ComponentManager& cmpManager) const
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
		assert(IsComponentDataRegisteredInternal(typeId));

		const size_t idx = userComponentListIndexForDataType_[typeId];
		assert(idx < UserComponentTypeList::size);

		const InlineStorage<kUserComponentStorageSize>& cmpStorage =
			std::invoke(UserComponentDispatchTable::kGetConstUserComponentStorage[idx],
						entity, cmpManager);

		return cmpStorage.Get<T>();
	}

	template <typename T>
	void RemoveComponentData(Entity_t entity, ComponentManager& cmpManager)
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
		if (!IsComponentDataRegisteredInternal(typeId))
		{
			return;
		}

		const size_t idx = userComponentListIndexForDataType_[typeId];
		assert(idx < UserComponentTypeList::size);

		std::invoke(UserComponentDispatchTable::kRemoveUserComponent[idx],
					entity, cmpManager);
	}

	template <typename T>
	bool HasComponentData(Entity_t entity, const ComponentManager& cmpManager) const
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
		if (!IsComponentDataRegisteredInternal(typeId))
		{
			return false;
		}

		const size_t idx = userComponentListIndexForDataType_[typeId];
		assert(idx < UserComponentTypeList::size);

		return std::invoke(UserComponentDispatchTable::kHasUserComponent[idx],
						   entity, cmpManager);
	}
	 
	template <typename T>
	bool IsComponentDataRegistered() const
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());

		return IsComponentDataRegisteredInternal(typeId);
	}

	template <typename T>
	ComponentSignature GetComponentDataSignature() const
	{
		const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
		if (!IsComponentDataRegisteredInternal(typeId))
		{
			return 0;
		}

		const size_t idx = userComponentListIndexForDataType_[typeId];
		assert(idx < UserComponentTypeList::size);

		return std::invoke(UserComponentDispatchTable::kGetUserComponentBit[idx]);
	}

private:
	bool IsComponentDataRegisteredInternal(size_t typeId) const
	{
		return typeId < userComponentListIndexForDataType_.size() &&
			userComponentListIndexForDataType_[typeId] < UserComponentTypeList::size;
	}

	bool RegisterComponentDataInternal(size_t typeId)
	{
		if (GetAvailableComponentCount() <= 0)
		{
			return false;
		}
		if (typeId >= userComponentListIndexForDataType_.size())
		{
			userComponentListIndexForDataType_.resize(typeId + 1, 
				std::numeric_limits<size_t>::max());
		}

		userComponentListIndexForDataType_[typeId] = nextFreeComponentIndex_++;

		return true;
	}

	std::vector<size_t> userComponentListIndexForDataType_;
	size_t nextFreeComponentIndex_ = 0;
};

