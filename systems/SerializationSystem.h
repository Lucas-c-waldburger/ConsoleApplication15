#pragma once
#include "System.h"

namespace nlohmann {
class json;
} // nlohmann

class SerializationSystem : public System
{
public:
	void Serialize(nlohmann::json& j);
	void Deserialize(const nlohmann::json& j);
};