#pragma once
#include "../scripting/ScriptManager.h"

class PhysicsSystem
{
public:
	void Update(float deltaTime);


private:
	void RunEntityScripts(ScriptManager& scriptManager);
};


