#include "GuiEditBasicComponents.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "GuiEditPropertyTable.h"

namespace ui {

bool GuiEdit(Transform& tf)
{
	ImGui::PushID("Transform");
	const bool changed = GuiEditClass(tf, "Transform", [](auto& tf) {
		bool b = GuiEdit(tf.position, "position");
		b |= GuiEdit(tf.rotation, "rotation");
		b |= GuiEdit(tf.scale, "scale", { .speed = 0.05f });
		return b;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(CameraTarget& ct)
{ 
	ImGui::PushID("CameraTarget");
	const bool changed = GuiEditClass(ct, "CameraTarget", [](auto& ct) {
		bool b = GuiEdit(ct.offset, "offset");
		b |= GuiEdit(ct.followSpeed, "followSpeed");
		b |= GuiEdit(ct.stopRadius, "stopRadius");
		return b;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(Parent& p)
{
	ImGui::PushID("Parent");
	const bool changed = GuiEditClass(p, "Parent", [](auto& p) {
		const std::string eStr = GuiGetEntityString(p.entityId);
		ImGui::LabelText("entityId", "%s", eStr.c_str());
		return false;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(Children& ch)
{
	ImGui::PushID("Children");
	const bool changed = GuiEditClass(ch, "Children", [](auto& ch) {
		const auto& es = ch.childEntityIds;
		GuiDrawContainer(es, "childEntityIds");
		return false;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(Tags& tg)
{
	ImGui::PushID("Tags");
	const bool changed = GuiEditClass(tg, "Tags", [](auto& tg) {
		const auto& tags = tg.tags;
		GuiDrawContainer(tags);

		char buffer[256] = {};

		ImGui::InputText("##Name", buffer, sizeof(buffer));
		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			tg.tags.insert(buffer);
			return true;
		}

		return false;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(Timer& tmr)
{
	ImGui::PushID("Timer");
	const bool changed = GuiEditClass(tmr, "Timer", [](auto& tmr) {
		bool b = GuiEdit(tmr.elapsed, "elapsed");
		b |= GuiEdit(tmr.duration, "duration");
		b |= GuiEdit(tmr.numRepeats, "numRepeats");
		return b;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(Name& nm)
{
	ImGui::PushID("Name");
	const bool changed = GuiEditClass(nm, "Name", [](auto& nm) {
		return GuiEdit(nm.value, "value");
	});
	ImGui::PopID();
	return changed;
}

// GUI EDIT PROPERTY //
bool GuiEditProperty(Transform& tf)
{
	bool b = Property("position", tf.position);
	b |= Property("rotation", tf.rotation);
	b |= Property("scale", tf.scale);
	return b;
}

bool GuiEditProperty(CameraTarget& ct)
{
	bool b = Property("offset", ct.offset);
	b |= Property("followSpeed", ct.followSpeed);
	b |= Property("stopRadius", ct.stopRadius);
	return b;
}

bool GuiEditProperty(Parent& p)
{
	const std::string eStr = GuiGetEntityString(p.entityId);
	GuiDrawProperty(eStr);
	return false;
}

bool GuiEditProperty(Children& ch)
{
	const auto& es = ch.childEntityIds;
	GuiDrawProperty(es);
	return false;
}

bool GuiEditProperty(Tags& tg)
{
	const auto& tags = tg.tags;
	GuiDrawProperty(tags);

	std::string entry;
	GuiEditProperty(entry);

	ImGui::SameLine();

	if (ImGui::Button("Add"))
	{
		tg.tags.insert(entry);
		return true;
	}

	return false;
}

bool GuiEditProperty(Timer& tmr)
{
	bool b = Property("elapsed", tmr.elapsed);
	b |= Property("duration", tmr.duration);
	b |= Property("numRepeats", tmr.numRepeats);
	return b;
}

bool GuiEditProperty(Name& nm)
{
	return Property("value", nm.value);
}

} // ui

#endif