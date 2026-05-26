#pragma once
#include "../../core/Bitset.h"
#include "../SerializationConcepts.h"

template <typename BasicJson>
void to_json(BasicJson& j, const EventDataBitset& evBitset)
{
	j = evBitset.GetBitset().to_string();
}
template <typename BasicJson>
void from_json(const BasicJson& j, EventDataBitset& evBitset)
{	
	evBitset = EventDataBitset{ EventDataBitset::BitsetType{j.get<std::string>()} };
}

template <typename BasicJson>
void to_json(BasicJson& j, const ComponentBitset& cmpBitset)
{
	j = cmpBitset.GetBitset();
}
template <typename BasicJson>
void from_json(const BasicJson& j, ComponentBitset& cmpBitset)
{
	cmpBitset = ComponentBitset{ j.get<ComponentBitset::BitsetType>() };
}