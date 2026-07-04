#include "GuiEditPhysics.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "GuiEditPropertyTable.h"

namespace ui {

bool GuiEdit(Force& f, const char* label)
{
	return GuiEditClass(f, label, [](auto& f) {
		bool b = GuiEdit(f.value, "value");
		b |= GuiEdit(f.worldPoint, "worldPoint");
		return b;
	});
}

bool GuiEdit(ForceRequests& fr, const char* label)
{
	return GuiEditClass(fr, label, [](auto& fr) {
		bool b = GuiEditContainer(fr.forces, "forces");
		b |= GuiEditContainer(fr.impulses, "impulses");
		return b;
	});
}

bool GuiEdit(BodyLimits& bl, const char* label)
{
	return GuiEditClass(bl, label, [](auto& bl) {
		bool b = GuiEdit(bl.linearVelocity, "linearVelocity");
		b |= GuiEdit(bl.angularVelocity, "angularVelocity");
		b |= GuiEdit(bl.maxImpulse, "maxImpulse");
		return b;
	});
}

// GUI EDIT PROPERTY //
bool GuiEditProperty(Force& f)
{
	bool b = Property("value", f.value);
	b |= Property("worldPoint", f.worldPoint);
	return b;
}

bool GuiEditProperty(ForceRequests& fr)
{
	bool b = PropertyGroup("forces", [&fr] { return Property("", fr.forces); });
	b |= PropertyGroup("impulses", [&fr] { return Property("", fr.impulses); });
	return b;
}

bool GuiEditProperty(BodyLimits& bl)
{
	bool b = Property("linearVelocity", bl.linearVelocity);
	b |= Property("angularVelocity", bl.angularVelocity);
	b |= Property("maxImpulse", bl.maxImpulse);
	return b;
}


} // ui

#endif