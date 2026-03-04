#pragma once
#include <ostream>
#include <istream>
#include <filesystem>
#include "GirlPhysicsEditor.h"
#include "../../../deps/nlohmann/json.hpp"

namespace test {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AnimationDeltas,
	idleTime,
	attackATime,
	attackBTime,
	landTime,
	landTimeEndMod,
	walkDeltaX,
	jumpDeltaY,
	fallDeltaY)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MoveTargets,
	jumpVelY,
	accelGround,
	accelAir,
	maxSpeed)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GirlPhysicsEditor::Data::InternalValues,
	colliderFriction,
	colliderLandingFriction,
	walkStopVelX)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GirlPhysicsEditor::Data,
	srcAnimDeltas,
	srcMoveTargets,
	internalValues)

class GirlStatConfig
{
public:

	static constexpr std::string_view kGirlConfigJsonFilename = "girl_config.json";
	static constexpr std::string_view kGirlConfigJsonKeyAnimDeltas = "animation_deltas";
	static constexpr std::string_view kGirlConfigJsonKeyMoveTargets = "move_targets";

	static bool ConfigFileExists()
	{
		return std::filesystem::exists(kGirlConfigJsonFilename);
	}

	static Result<Void> SerializeToJson(const AnimationDeltas* animDeltas,
										const MoveTargets* targets)
	{
		if (!(animDeltas || targets))
		{
			return kVoid;
		}

		nlohmann::json j;

		// if we're only serializing one component, use the orig data in the json for the other
		if (ConfigFileExists())
		{
			std::ifstream file(std::string{ kGirlConfigJsonFilename });
			if (!file)
			{
				return MAKE_ERROR_FMT("Could not open exisiting JSON "
					" file at path: '{}'", kGirlConfigJsonFilename);
			}

			try
			{
				j = nlohmann::json::parse(file);
			}
			catch (const nlohmann::json::parse_error& err)
			{
				return MAKE_ERROR_FMT("JSON parse error: '{}'", err.what());
			}
		}

		if (animDeltas)
		{
			j[kGirlConfigJsonKeyAnimDeltas] = *animDeltas;
		}
		if (targets)
		{
			j[kGirlConfigJsonKeyMoveTargets] = *targets;
		}

		std::ofstream file(std::string{ kGirlConfigJsonFilename });
		if (!file)
		{
			return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'",
				kGirlConfigJsonFilename);
		}

		file << std::setw(4) << j;

		return kVoid;
	}

	static Result<Void> DeserializeFromJson(AnimationDeltas* animDeltas,
											MoveTargets* targets)
	{
		if (!(animDeltas || targets))
		{
			return kVoid;
		}

		std::ifstream file(std::string{ kGirlConfigJsonFilename });
		if (!file)
		{
			return MAKE_ERROR_FMT("Could not open exisiting JSON "
				" file at path: '{}'", kGirlConfigJsonFilename);
		}

		nlohmann::json j;
		try
		{
			j = nlohmann::json::parse(file);
		}
		catch (const nlohmann::json::parse_error& err)
		{
			return MAKE_ERROR_FMT("JSON parse error: '{}'", err.what());
		}

		if (animDeltas)
		{
			if (!j.contains(kGirlConfigJsonKeyAnimDeltas))
			{
				return MAKE_ERROR_FMT("Config did not have field named '{}'",
					kGirlConfigJsonKeyAnimDeltas);
			}

			from_json(j.at(kGirlConfigJsonKeyAnimDeltas), *animDeltas);
		}

		if (targets)
		{
			if (!j.contains(kGirlConfigJsonKeyMoveTargets))
			{
				return MAKE_ERROR_FMT("Config did not have field named '{}'",
					kGirlConfigJsonKeyMoveTargets);
			}

			from_json(j.at(kGirlConfigJsonKeyMoveTargets), *targets);
		}

		return kVoid;
	}

private:
	GirlStatConfig() = default;
};

} // test