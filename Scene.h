#pragma once
#include "systems/SystemRegistry.h"
#include "test/Fixtures.h"

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

class GrapplePhysicsScene
{
public:
	static Result<Void> Run();
};

class ChainScene
{
public:
	static Result<Void> Run(std::shared_ptr<SceneFixture> scene);
};

class TextScene
{
public:
	static Result<Void> Run(std::shared_ptr<SceneFixture> scene);
};

class SpriteScene
{
public:
	static Result<Void> Run(std::shared_ptr<SceneFixture> scene);
};

