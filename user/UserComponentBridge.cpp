#include "UserComponentBridge.h"

/** UserComponentSerializationHelper */
void UserComponentSerializationHelper::SerializeComponentData(nlohmann::json& j, Entity_t e, 
															  const UserComponentBridge& bridge,
															  const ComponentManager& cmpManager) const
{
	assert(userComponentNames_.size() == serializeFns_.size());
	for (size_t i = 0; i < serializeFns_.size(); ++i)
	{
		if (serializeFns_[i])
		{
			std::invoke(serializeFns_[i], j, e, userComponentNames_[i], bridge, cmpManager);
		}
	}
}

Result<Void> UserComponentSerializationHelper::DeserializeComponentData(const nlohmann::json& j, Entity_t e, 
																		UserComponentBridge& bridge,
																		ComponentManager& cmpManager)
{
	assert(userComponentNames_.size() == deserializeFns_.size());
	for (size_t i = 0; i < deserializeFns_.size(); ++i)
	{
		if (deserializeFns_[i])
		{
			TRY(std::invoke(deserializeFns_[i], j, e, userComponentNames_[i], bridge, cmpManager));
		}
	}

	return kVoid;
}

void UserComponentSerializationHelper::Reset()
{
	serializeFns_.clear();
	deserializeFns_.clear();
	userComponentNames_.clear();
}

/** UserComponentBridge */
size_t UserComponentBridge::GetAvailableComponentCount() const
{
	assert(nextFreeComponentIndex_ <= UserComponentTypeList::size);

	return (nextFreeComponentIndex_ >= UserComponentTypeList::size)
		? 0
		: UserComponentTypeList::size - nextFreeComponentIndex_;
}

bool UserComponentBridge::IsComponentDataRegisteredInternal(size_t typeId) const
{
	return typeId < userComponentListIndexForDataType_.size() &&
		   userComponentListIndexForDataType_[typeId] < UserComponentTypeList::size;
}

bool UserComponentBridge::RegisterComponentDataInternal(size_t typeId)
{
	if (GetAvailableComponentCount() <= 0)
	{
		return false;
	}
	if (typeId >= userComponentListIndexForDataType_.size())
	{
		userComponentListIndexForDataType_.resize(
			typeId + 1, std::numeric_limits<size_t>::max()
		);
	}

	userComponentListIndexForDataType_[typeId] = nextFreeComponentIndex_++;

	return true;
}

void UserComponentBridge::SerializeComponentData(nlohmann::json& j, Entity_t e,
												 const ComponentManager& cmpManager) const
{
	serializationHelper_.SerializeComponentData(j, e, *this, cmpManager);
}

Result<Void> UserComponentBridge::DeserializeComponentData(const nlohmann::json& j, Entity_t e,
														   ComponentManager& cmpManager)
{
	return serializationHelper_.DeserializeComponentData(j, e, *this, cmpManager);
}

void UserComponentBridge::Reset()
{
	userComponentListIndexForDataType_.clear();
	nextFreeComponentIndex_ = 0;
	serializationHelper_.Reset();
}