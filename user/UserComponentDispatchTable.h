#pragma once
#include "../components/UserComponents.h"
#include "../ecs/ComponentManager.h"

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

	using AddGetUserComponentStorageSig = 
		InlineStorage<kUserComponentStorageSize>&(*)(Entity_t, ComponentManager&);

	template <typename T>
	static const InlineStorage<kUserComponentStorageSize>&
	GetConstUserComponentStorageImpl(Entity_t entity, const ComponentManager& cmpManager)
	{
		assert(cmpManager.HasComponent<T>(entity));

		return cmpManager.GetComponent<T>(entity).data;
	}

	using GetConstUserComponentStorageSig =
		const InlineStorage<kUserComponentStorageSize>&(*)(Entity_t, const ComponentManager&);

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

	template <typename TList>
	struct inner
	{
		template <typename T>
		ComponentId GetUserComponentIdImpl()
		{
			return MakeComponentId<T, TList>();
		}

		using GetUserComponentIdSig = ComponentId(*)();

		static constexpr GetUserComponentIdSig kGetUserComponentId[] = {
			&GetUserComponentIdImpl<Ts>...
		};
	};
};

using UserComponentDispatchTable = UserComponentDispatchTableImpl<UserComponentTypeList>;
