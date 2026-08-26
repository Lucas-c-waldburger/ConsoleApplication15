#pragma once
#include <vector>
#include <unordered_map>
#include <bit>
#include "../serial/SerializationConcepts.h"
#include "UserComponentTypeId.h"
#include "UserComponentDispatchTable.h"
#include "../core/TypeInfo.h"

/** UserComponentSerializationHelper */
class UserComponentSerializationHelper
{
public:
	friend class UserComponentBridge;

	using SerializeFn = void(*)(nlohmann::json&, Entity_t, std::string_view,
								const UserComponentBridge&, const ComponentManager&);

	using DeserializeFn = Result<Void>(*)(const nlohmann::json&, Entity_t, std::string_view, 
										  UserComponentBridge&, ComponentManager&);

	template <typename T>
	void HandleSerializerRegistration(std::string_view cmpName);

	void SerializeComponentData(nlohmann::json& j, Entity_t e,
								const UserComponentBridge& bridge,
								const ComponentManager& cmpManager) const;

	Result<Void> DeserializeComponentData(const nlohmann::json& j, Entity_t e,
										  UserComponentBridge& bridge, ComponentManager& cmpManager);

	void Reset();

private:
	std::vector<SerializeFn> serializeFns_;
	std::vector<DeserializeFn> deserializeFns_;
	std::vector<std::string> userComponentNames_;
};

class UserComponentBridge
{
public:
	size_t GetAvailableComponentCount() const;

	template <typename T>
	bool RegisterComponentData(std::string_view cmpName);

	template <typename T>
	bool RegisterComponentData();

	template <typename T, typename...Args> requires std::constructible_from<T, Args...>
	T& AddComponentData(Entity_t entity, ComponentManager& cmpManager, Args&&...args);

	template <typename T>
	T& GetComponentData(Entity_t entity, ComponentManager& cmpManager);

	template <typename T>
	const T& GetComponentData(Entity_t entity, const ComponentManager& cmpManager) const;

	template <typename T>
	void RemoveComponentData(Entity_t entity, ComponentManager& cmpManager);

	template <typename T>
	bool HasComponentData(Entity_t entity, const ComponentManager& cmpManager) const;
	 
	template <typename T>
	bool IsComponentDataRegistered() const;

	template <typename T>
	ComponentSignature GetComponentDataSignature() const;

	template <typename T, typename TList>
	ComponentId GetComponentDataId() const;

	void SerializeComponentData(nlohmann::json& j, Entity_t e,
								const ComponentManager& cmpManager) const;

	Result<Void> DeserializeComponentData(const nlohmann::json& j, Entity_t e,
										  ComponentManager& cmpManager);

	void Reset();

private:
	bool IsComponentDataRegisteredInternal(size_t typeId) const;
	bool RegisterComponentDataInternal(size_t typeId);

	std::vector<size_t> userComponentListIndexForDataType_;
	size_t nextFreeComponentIndex_ = 0;
	UserComponentSerializationHelper serializationHelper_;
};

/** UserComponentSerializer template definitions */
template <typename T>
void UserComponentSerializationHelper::HandleSerializerRegistration(std::string_view cmpName)
{
	if constexpr (HasToJson<T> || HasFromJson<T>)
	{
		userComponentNames_.emplace_back((cmpName.empty())
			? std::string{ TypeInfo<T>::name }
			: std::string{ cmpName }
		);

		if constexpr (HasToJson<T>)
		{
			static constexpr auto sfn = 
			+[](nlohmann::json& j, Entity_t e, std::string_view name,
				const UserComponentBridge& bridge, const ComponentManager& cmpManager) {
				if (!bridge.HasComponentData<T>(e, cmpManager))
				{
					return;
				}

				const auto& cmpData = bridge.GetComponentData<T>(e, cmpManager);

				to_json(j[name], cmpData);
			};

			serializeFns_.emplace_back(sfn);
		}
		else
		{
			serializeFns_.emplace_back(nullptr);
		}

		if constexpr (HasFromJson<T>)
		{
			static constexpr auto dfn = 
			+[](const nlohmann::json& j, Entity_t e, std::string_view name, 
				UserComponentBridge& bridge, ComponentManager& cmpManager) -> Result<Void> {
				if (!j.contains(name))
				{
					return kVoid;
				}

				auto& cmpData = bridge.AddComponentData<T>(e, cmpManager);

				try
				{
					from_json(j.at(name), cmpData);
				}
				catch (const nlohmann::json::exception& err)
				{
					return MAKE_ERROR_FMT("Could not deserialize component '{}': {}",
						name, err.what());
				}

				return kVoid;
			};

			deserializeFns_.emplace_back(dfn);
		}
		else
		{
			deserializeFns_.emplace_back(nullptr);
		}
	}	
}
/**/

/** UserComponentBridge template definitions */
template <typename T>
bool UserComponentBridge::RegisterComponentData(std::string_view cmpName)
{
	const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
	if (IsComponentDataRegisteredInternal(typeId))
	{
		return true;
	}
	 
	const bool registered = RegisterComponentDataInternal(typeId);
	if (!registered)
	{
		return false;
	}

	serializationHelper_.HandleSerializerRegistration<T>(cmpName);

	return true;
}

template <typename T>
bool UserComponentBridge::RegisterComponentData()
{
	return RegisterComponentData<T>({});
}

template <typename T, typename...Args> requires std::constructible_from<T, Args...>
T& UserComponentBridge::AddComponentData(Entity_t entity, ComponentManager& cmpManager, Args&&...args)
{
	const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());

	if (!IsComponentDataRegisteredInternal(typeId))
	{
		const bool registered = RegisterComponentDataInternal(typeId);
		assert(registered);
	}

	const size_t idx = userComponentListIndexForDataType_[typeId];
	assert(idx < UserComponentTypeList::size);

	const bool hasCmp = std::invoke(UserComponentDispatchTable::kHasUserComponent[idx],
		entity, cmpManager);
	if (hasCmp)
	{
		return std::invoke(UserComponentDispatchTable::kGetUserComponentStorage[idx],
			entity, cmpManager).Get<T>();
	}

	InlineStorage<kUserComponentStorageSize>& newCmpStorage =
		std::invoke(UserComponentDispatchTable::kAddUserComponentStorage[idx],
			entity, cmpManager);

	return newCmpStorage.Emplace<T>(std::forward<Args>(args)...);
}

template <typename T>
T& UserComponentBridge::GetComponentData(Entity_t entity, ComponentManager& cmpManager)
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
const T& UserComponentBridge::GetComponentData(Entity_t entity, const ComponentManager& cmpManager) const
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
void UserComponentBridge::RemoveComponentData(Entity_t entity, ComponentManager& cmpManager)
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
bool UserComponentBridge::HasComponentData(Entity_t entity, const ComponentManager& cmpManager) const
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
bool UserComponentBridge::IsComponentDataRegistered() const
{
	const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());

	return IsComponentDataRegisteredInternal(typeId);
}

template <typename T>
ComponentSignature UserComponentBridge::GetComponentDataSignature() const
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

template <typename T, typename TList>
inline ComponentId UserComponentBridge::GetComponentDataId() const
{
	const auto typeId = static_cast<size_t>(GetUserComponentTypeId<T>());
	if (!IsComponentDataRegisteredInternal(typeId))
	{
		return {};
	}

	const size_t idx = userComponentListIndexForDataType_[typeId];
	assert(idx < UserComponentTypeList::size);

	return std::invoke(UserComponentDispatchTable::template inner<TList>::kGetUserComponentId[idx]);
}
/**/