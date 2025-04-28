#include "Scene.h"
#include "core/Logger.h"
#include "sdl/SDLite.h"
#include "sdl/SDLUtils.h"
#include "ecs/Ecs.h"
#include "physics/B2World.h"

namespace {
    static constexpr SDL_FPoint kGroundPosition = {
        static_cast<float>(SDLite::kWindowWidth) / 2.0f,
        static_cast<float>(SDLite::kWindowHeight) - 20.0f
    };

    static constexpr SDL_FPoint kScreenCenterPosition = {
        static_cast<float>(SDLite::kWindowWidth) / 2.0f,
        static_cast<float>(SDLite::kWindowHeight) / 2.0f
    };

    Result<B2Body> AddGroundBody(B2World& world)
    {
        assert(world.IsValid());

        B2ShapeDefinition shapeDef{};
        shapeDef.type = B2Shape::Type::Polygon;
        shapeDef.data = {
            .dimensions = Dimensions<float>{ SDLite::kWindowWidth, 20.0f }
        };

        B2BodyDefinition bodyDef{};
        bodyDef.bodyData.type = b2_staticBody;
        bodyDef.bodyData.position = ToB2VecScaled(kGroundPosition);
       
        bodyDef.shapeDatas.push_back(std::move(shapeDef));

        return world.AddBody(bodyDef);
    }

    Result<B2Body> AddDynamicBody(B2World& world)
    {
        assert(world.IsValid());

        B2BodyDefinition bodyDef{};
        bodyDef.bodyData.type = b2_dynamicBody;
        bodyDef.bodyData.position = ToB2VecScaled(kScreenCenterPosition);
        
        return world.AddBody(bodyDef);
    }

    Result<B2Shape> AddPolyToDynamicBody(B2Body& dynamicBody)
    {
        B2ShapeDefinition shapeDef{};
        shapeDef.type = B2Shape::Type::Polygon;
        shapeDef.data = {
            .dimensions = Dimensions<float>{ 50.0f, 50.0f },
        };
        shapeDef.def.density = 1.0f;
        shapeDef.def.material.friction = 0.3f;
        shapeDef.def.material.restitution = 1.0f;

        return dynamicBody.AddShape(shapeDef);
    }



}

Result<Void> B2Scene::Run()
{
	Logger::StartSession();
	SDLite::Start();

    B2World world = B2World::Create(0, 9.8f);

    TRY(AddGroundBody(world), groundBody);
    TRY(AddDynamicBody(world), dynamicBody);
    TRY(AddPolyToDynamicBody(dynamicBody), dynamicPolyShape);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;
    
    SDL_Event ev;
    while (true)
    {
        SDL_FPoint forceNewtons = { 0.0 };

        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                break;
            }
            if (ev.type == SDL_KEYDOWN)
            {
                switch (ev.key.keysym.sym) 
                {
                case SDLK_LEFT:
                    forceNewtons.x -= 3.0f;
                    break;
                case SDLK_RIGHT:
                    forceNewtons.x += 3.0f;
                    break;
                case SDLK_UP:
                    forceNewtons.y -= 3.0f;
                    break;
                case SDLK_DOWN:
                    forceNewtons.y += 3.0f;
                    break;
                default:
                    break;
                }
            }
        }

        dynamicBody.ApplyLinearImpulse(forceNewtons, forceNewtons);

        world.Step(timeStep, subStepCount);

        SDL_FPoint dynamicPos = dynamicBody.GetPosition();
        float dynamicAngle = dynamicBody.GetAngle();

        LOG_INFO_FMT("position = [{:.2f}, {:.2f}], rotation = {:.2f}",
            dynamicPos.x, dynamicPos.y, dynamicAngle);

        SDLite::Renderer().Clear();

        assert(dynamicPolyShape.GetShapeType() == B2Shape::Type::Polygon);
        auto dynamicPolyVerts = dynamicPolyShape.GetAs<B2PolygonShape>().GetVertices();

        auto origColor = GetRenderDrawColor(SDLite::Renderer());
        SetRenderDrawColor(SDLite::Renderer(), SDLite::kColorBlack);

        SDL_RenderDrawLinesF(SDLite::Renderer(), dynamicPolyVerts.data(), dynamicPolyVerts.size());

        SetRenderDrawColor(SDLite::Renderer(), origColor);

        SDLite::Renderer().Show();
    }

    world.Destroy();

    Logger::EndSession();
    SDLite::Exit();

	return Void{};
}
