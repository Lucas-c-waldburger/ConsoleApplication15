#pragma once
#include "systems/SystemRegistry.h"
#include "atlas/AtlasManager.h"

class B2Scene
{
public:
	static Result<Void> Run();
};

class SimplePhysicsScene
{
public:
	static Result<Void> Run();
};



class Scene
{
public:
	using MainLoopFn = Result<Void>(*)(Scene&);

private:
	SystemRegistry systemRegistry_;
	impl::AtlasStore atlasStore_;
	MainLoopFn mainLoop_ = nullptr;
};