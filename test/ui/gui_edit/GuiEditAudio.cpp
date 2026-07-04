#include "GuiEditAudio.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "GuiEditPropertyTable.h"

namespace ui {

bool GuiEdit(AudioSpatialData& asd, const char* label)
{
	return GuiEditClass(asd, label, [](auto& asd) {
		bool b = GuiEdit(asd.angle, "angle");
		b |= GuiEdit(asd.distance, "distance");
		b |= GuiEdit(asd.panning, "panning");
		return b;
	});
}

bool GuiEdit(AudioFadeMs& af, const char* label)
{
	return GuiEditClass(af, label, [](auto& af) {
		bool b = GuiEdit(af.in, "in");
		b |= GuiEdit(af.out, "out");
		return b;
	});
}

bool GuiEdit(AudioPlayCommand& apc, const char* label)
{
	static constexpr const char* kNames[] = {
		"None",
		"Pause",
		"Resume",
		"Restart",
		"Stop"
	};
	int cur = apc == AudioPlayCommand::None ? 0 :
			  apc == AudioPlayCommand::Pause ? 1 :
			  apc == AudioPlayCommand::Resume ? 2 :
			  apc == AudioPlayCommand::Restart ? 3 : 4;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: apc = AudioPlayCommand::None; break;
		case 1: apc = AudioPlayCommand::Pause; break;
		case 2: apc = AudioPlayCommand::Resume; break;
		case 3: apc = AudioPlayCommand::Restart; break;
		case 4: apc = AudioPlayCommand::Stop; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(AudioStatus& as, const char* label)
{
	static constexpr const char* kNames[] = {
		"Playing",
		"Paused",
		"Stopping",
		"Stopped",
		"Staged"
	};
	int cur = as == AudioStatus::Playing ? 0 :
			  as == AudioStatus::Paused ? 1 :
			  as == AudioStatus::Stopping ? 2 :
			  as == AudioStatus::Stopped ? 3 : 4;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: as = AudioStatus::Playing; break;
		case 1: as = AudioStatus::Paused; break;
		case 2: as = AudioStatus::Stopping; break;
		case 3: as = AudioStatus::Stopped; break;
		case 4: as = AudioStatus::Staged; break;
		}

		return true;
	}
	return false;
}

//template <template <typename> class Wrap>
//bool GuiEdit(AudioSettingsTemplate<Wrap>& ast, const char* label)
//{
//	return GuiEditClass(ast, label, [](auto& ast) {
//		return GuiEdit(ast.volume, "volume") ||
//			   GuiEdit(ast.loopCount, "loopCount") ||
//			   GuiEdit(ast.fadeMs, "fadeMs") ||
//			   GuiEdit(ast.spatial, "spatial") ||
//			   GuiEdit(ast.trackPosition, "trackPosition");
//	});
//}

bool GuiEdit(AudioChannelSettings& acs, const char* label)
{
	return GuiEditClass(acs, label, [](auto& acs) {
		bool b = GuiEdit(acs.volume, "volume");
		b |= GuiEdit(acs.loopCount, "loopCount");
		b |= GuiEdit(acs.fadeMs, "fadeMs");
		b |= GuiEdit(acs.spatial, "spatial");
		b |= GuiEdit(acs.trackPosition, "trackPosition");
		return b;
	});
}
bool GuiEdit(AudioUpdateSettings& aus, const char* label)
{
	return GuiEditClass(aus, label, [](auto& aus) {
		bool b = GuiEdit(aus.volume, "volume");
		b |= GuiEdit(aus.loopCount, "loopCount");
		b |= GuiEdit(aus.fadeMs, "fadeMs");
		b |= GuiEdit(aus.spatial, "spatial");
		b |= GuiEdit(aus.trackPosition, "trackPosition");
		return b;
	});
}

bool GuiEdit(NewAudioRequest& nar, const char* label)
{
	return GuiEditClass(nar, label, [](auto& nar) {
		GuiDraw(nar.audioHandle, "audioHandle");
		bool b = GuiEdit(nar.settings, "settings");
		b |= GuiEdit(nar.force, "force");
		return b;
	});
}

bool GuiEdit(AudioUpdateRequest& aur, const char* label)
{
	return GuiEditClass(aur, label, [](auto& aur) {
		GuiDraw(aur.instanceId, "instanceId");
		bool b = GuiEdit(aur.command, "command");
		b |= GuiEdit(aur.settings, "settings");
		b |= GuiEdit(aur.spatialData, "spatialData");
		return b;
	});
}

bool GuiEdit(ActiveAudio& aa, const char* label)
{
	ImGui::PushID("ActiveAudio");
	const bool changed = GuiEditClass(aa, label, [](auto& aa) {
		GuiDraw(aa.audioHandle, "audioHandle");
		GuiDraw(aa.instanceId, "instanceId");
		bool b = GuiEdit(aa.status, "status");
		b |= GuiEdit(aa.onChannel, "onChannel");
		b |= GuiEdit(aa.settings, "settings");
		return b;
	});
	ImGui::PopID();
	return changed;
}

// GUI EDIT PROPERTY //

bool GuiEditProperty(AudioSpatialData& asd)
{
	bool changed = Property("angle", asd.angle);
	changed |= Property("distance", asd.distance);
	changed |= Property("panning", asd.panning);
	return changed;
}
 
bool GuiEditProperty(AudioFadeMs& af)
{
	return GuiEditProperties<"in", "out">(af.in, af.out);
}

bool GuiEditProperty(AudioPlayCommand& apc)
{
	static constexpr const char* kNames[] = {
		"None",
		"Pause",
		"Resume",
		"Restart",
		"Stop"
	};
	int cur = apc == AudioPlayCommand::None ? 0 :
		apc == AudioPlayCommand::Pause ? 1 :
		apc == AudioPlayCommand::Resume ? 2 :
		apc == AudioPlayCommand::Restart ? 3 : 4;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: apc = AudioPlayCommand::None; break;
		case 1: apc = AudioPlayCommand::Pause; break;
		case 2: apc = AudioPlayCommand::Resume; break;
		case 3: apc = AudioPlayCommand::Restart; break;
		case 4: apc = AudioPlayCommand::Stop; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(AudioStatus& as)
{
	static constexpr const char* kNames[] = {
		"Playing",
		"Paused",
		"Stopping",
		"Stopped",
		"Staged"
	};
	int cur = as == AudioStatus::Playing ? 0 :
		as == AudioStatus::Paused ? 1 :
		as == AudioStatus::Stopping ? 2 :
		as == AudioStatus::Stopped ? 3 : 4;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: as = AudioStatus::Playing; break;
		case 1: as = AudioStatus::Paused; break;
		case 2: as = AudioStatus::Stopping; break;
		case 3: as = AudioStatus::Stopped; break;
		case 4: as = AudioStatus::Staged; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(AudioChannelSettings& acs)
{
	bool changed = Property("volume", acs.volume);
	changed |= Property("loopCount", acs.loopCount);
	changed |= Property("fadeMs", acs.fadeMs);
	changed |= PropertyGroup("spatial", [&acs] { return Property("", acs.spatial); });
	changed |= Property("trackPosition", acs.trackPosition);
	return changed;
}

bool GuiEditProperty(AudioUpdateSettings& aus)
{
	bool changed = Property("volume", aus.volume);
	changed |= Property("loopCount", aus.loopCount);
	changed |= Property("fadeMs", aus.fadeMs);
	changed |= PropertyGroup("spatial", [&aus] { return Property("", aus.spatial); });
	changed |= Property("trackPosition", aus.trackPosition);
	return changed;
}

bool GuiEditProperty(NewAudioRequest& nar)
{
	const auto& h = nar.audioHandle;
	Property("audioHandle", h);

	bool changed = PropertyGroup("settings", [&nar] { return Property("", nar.settings); });
	changed |= Property("force", nar.force);
	return changed;
}

bool GuiEditProperty(AudioUpdateRequest& aur)
{
	const auto& h = aur.instanceId;
	Property("instanceId", h);

	bool changed = Property("command", aur.command);
	changed |= PropertyGroup("settings", [&aur] { return Property("", aur.settings); });
	changed |= PropertyGroup("spatialData", [&aur] { return Property("", aur.spatialData); });
	return changed;
}

bool GuiEditProperty(ActiveAudio& aa)
{
	const auto& ah = aa.audioHandle;
	const auto& ii = aa.instanceId;
	Property("audioHandle", ah);
	Property("instanceId", ii);

	bool changed = Property("status", aa.status);
	changed |= Property("onChannel", aa.onChannel);
	changed |= PropertyGroup("settings", [&aa] { return Property("", aa.settings); });
	return changed;
}

} // ui

#endif