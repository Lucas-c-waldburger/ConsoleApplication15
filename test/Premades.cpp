#include "Premades.h"
#include <ranges>
#include "../events/data/EventDataIncludes.h"
#include "../file/FilePathUtility.h"
#include "ui/Workspace.h"
#include "Fixtures.h"


auto ui::PointDrawHandler::GetLayPointCallback()
{
    return [this](const events::MouseInput& ev)
        {
            if (ev.input.state != InputState::Pressed) { return; }

            auto pos = ev.input.value.cursor.position.absolute;
            points_.emplace_back(pos);
        };
}

auto ui::PointDrawHandler::GetErasePointCallback()
{
    return [this](const events::MouseInput& ev)
        {
            if (ev.input.state != InputState::Pressed) { return; }

            points_.pop_back();
        };
}


//Result<ui::ColliderBoxMaker> 
//ui::ColliderBoxMaker::Create(SDL_Renderer* renderer, 
//    EventBus2& bus, Result<std::string>&& spritePath)
//{
//    assert(renderer);
//    if (!spritePath.Success())
//    {
//        return spritePath.GetError();
//    }
//
//    TRY(SpriteAtlas::Create(renderer), temp);
//    TRY(temp.LoadSprite(renderer, { .filepath = std::move(spritePath.GetValue()) }), sprite);
//
//    ui::ColliderBoxMaker maker{};
//
//    TRY(maker.textureRepo_.AttachAtlas(std::move(temp)), atlas);
//    assert(atlas);
//
//    auto& ent = maker.self_;
//    ent = ECS::CreateEntity();
//    assert(ent.IsValid());
//
//    ent.AddComponent(Transform{ .position = SDLite::kFWindowCenter });
//    ent.AddComponent(SpriteRenderableComponent{
//        .sprite = std::move(sprite),
//        .profile = { .debugDraw = { .boundingBox = { .on = true }}}
//    });
//
//    auto& tks = ent.AddComponent(SignalTokenStorage{}).signalTokens;
//    tks.emplace_back(bus.ConnectToInput(MouseInputSource::LeftButton,
//        [this](const events::MouseInputEvent& ev) {
//            if (ev.state != InputState::)
//        }
//    ));
//}



//test::ParticleEmitter::~ParticleEmitter()
//{
//    emitter_.Destroy();
//    for (auto& [particle, _] : particles_)
//    {
//        particle.Destroy();
//    }
//}
//
//void test::ParticleEmitter::AddParticle()
//{
//    if (liveCount_ >= maxParticles_)
//    {
//        return;
//    }
//
//    if (!bxGenerator_)
//    {
//        LOG_ERROR("No particle behavior generator set");
//        return;
//    }
//
//    auto& [particle, bx] = particles_[liveCount_++];
//
//    if (!particle.IsValid())
//    {
//        particle = ECS::CreateEntity();
//
//        // stop them from firing when destroyed or clogging up timer events
//        particle.SetEventProduction<events::TimerFired>(false);
//        particle.SetEventProduction<events::EntityDestroyed>(false);
//    }
//
//    particle.SetComponentVisibility(true);
//
//    bx = bxGenerator_();
//    assert(bx.lifetime > 0.0f);
//
//    const SDL_FPoint emitterPos = emitter_.GetComponent<Transform>().position;
//    particle.AddComponent<Transform>().position = emitterPos + bx.offset;
//
//    auto& srcRenderData = emitter_.GetComponent<Renderable>().renderData;
//    auto& renderable = particle.AddComponent<Renderable>();
//    renderable.renderData = srcRenderData;
//    renderable.profile.debugDraw.boundingBox.on = false;
//
//    auto& timer = particle.AddComponent<Timer>();
//    timer.duration = bx.lifetime;
//    timer.flags = Timer::Flag::Active;
//}
//
//void test::ParticleEmitter::Update(float delta)
//{
//    auto& spawnTimer = emitter_.GetComponent<Timer>();
//    if ((spawnTimer.flags & Timer::Flag::Active) == 0)
//    {
//        AddParticle();
//        spawnTimer.flags = Timer::Flag::Active;
//    }
//
//    if (liveCount_ <= 0)
//    {
//        return;
//    }
//
//    auto kill = [this](Entity& particle, size_t& i) {
//        // wont be processed by any system, will be considered IsValid()
//        particle.SetComponentVisibility(false);
//
//        if (--liveCount_ <= i)
//        {
//            return;
//        }
//
//        std::swap(particles_[i], particles_[liveCount_]);
//        --i;
//    };
//
//    auto emitterPos = emitter_.GetComponent<Transform>().position;
//
//    for (size_t i = 0; i < liveCount_; i++)
//    {
//        bool particleValid = UpdateParticle(particles_[i], delta);
//        if (!particleValid)
//        {
//            kill(particles_[i].first, i);
//        }
//    }
//}
//
//bool test::ParticleEmitter::UpdateParticle(std::pair<Entity, ParticleBehavior>& particleBx,
//                                           float delta)
//{
//    auto& [particle, bx] = particleBx;
//
//    if (!particle.IsValid())
//    {
//        return false;
//    }
//
//    auto [tf, timer, rend] = particle.GetComponents<Transform, Timer, Renderable>();
//
//    if ((timer.flags & Timer::Flag::Active) == 0)
//    {
//        return false;
//    }
//
//    const float t = timer.elapsed / timer.duration;
//
//    tf.position = bx.position.Evaluate(tf.position, delta, t);
//    tf.scale = bx.scale.Evaluate(tf.scale, delta, t);
//    tf.rotation = bx.rotation.Evaluate(tf.rotation, delta, t);
//
//    auto& mods = rend.profile.mods;
//    mods.color = bx.color.Evaluate(mods.color, delta, t);
//    mods.alpha = bx.alpha.Evaluate(mods.alpha, delta, t);
//
//    LOG_DEBUG_FMT("R:{} G:{} B:{} A:{}", 
//        mods.color.r, mods.color.g, mods.color.b, mods.alpha);
//
//    return true;
//}

auto SwordHandler::MakeTimerCallback()
{
    return [this](const events::TimerFired& ev)
    {
        if (activeFrame_ == kInactive) { return; }
        if (ev.producer != coordinator_.GetID()) { return; }

        int8_t lastFrame = activeFrame_;
        activeFrame_ = (lastFrame + 1 >= swordFrames_.size())
            ? kInactive
            : lastFrame + 1;

        if (activeFrame_ == kInactive) { return coordinator_.RemoveComponent<Timer>(); }

        swordFrames_[lastFrame].SetComponentVisibility<SpriteRenderableComponent>(false);

        auto& activeEnt = swordFrames_[activeFrame_];
        activeEnt.SetComponentVisibility<SpriteRenderableComponent>(true);

        auto parentEnt = ECS::GetEntityByID(bodyParentId_);
        if (!parentEnt.IsValid()) { return; }
        if (!parentEnt.HasComponent<Transform>()) { return; }
        if (!parentEnt.HasComponent<RigidBody>()) { return; }
        
        const auto& parentTf = parentEnt.GetComponent<Transform>();

        auto parentBBox = GetSpriteEntityBoundingBox(parentEnt);


        auto& activeTf = swordFrames_[activeFrame_].GetComponent<Transform>();
        auto& activeCollider = swordFrames_[activeFrame_].GetComponent<Collider>().shape;
    };
}

SDL_FRect SwordHandler::GetParentBodyBoundingBox()
{
    auto parentEnt = ECS::GetEntityByID(bodyParentId_);
    if (!parentEnt.IsValid()) { return {}; }
    if (!parentEnt.HasComponents<Transform, SpriteRenderableComponent>()) { return {}; }

    auto [tf, r] = parentEnt.GetComponents<Transform, SpriteRenderableComponent>();

    float w = r.sprite.plot.rect.w * tf.scale.x;
    float h = r.sprite.plot.rect.h * tf.scale.y;

    return { tf.position.x - (w / 2.0f), tf.position.y - (h / 2.0f), w, h };
}

SDL_FRect SwordHandler::GetSpriteEntityBoundingBox(const Entity& e)
{
    if (!e.IsValid()) { return {}; }
    if (!e.HasComponents<Transform, SpriteRenderableComponent>()) { return {}; }

    auto [tf, r] = e.GetComponents<Transform, SpriteRenderableComponent>();

    float w = r.sprite.plot.rect.w * tf.scale.x;
    float h = r.sprite.plot.rect.h * tf.scale.y;

    return { tf.position.x - (w / 2.0f), tf.position.y - (h / 2.0f), w, h };
}

Transform SwordHandler::TranslateBoundingBoxToTransform(const SDL_FRect& bbox)
{
    return Transform();
}

//SDL_FPoint SwordHandler::GetTransformPositionForSword(const Entity& parentEnt, 
//                                                      Entity& swordEnt)
//{
//    assert(parentEnt.IsValid());
//    assert((parentEnt.HasComponents<Transform, SpriteRenderableComponent>()));
//    assert(swordEnt.IsValid());
//    assert((swordEnt.HasComponents<Transform, SpriteRenderableComponent>()));
//
//    auto [parentTf, parentR] = parentEnt.GetComponents<Transform, SpriteRenderableComponent>();
//    auto [swordTf, swordR] = swordEnt.GetComponents<Transform, SpriteRenderableComponent>();
//
//    swordTf.scale = parentTf.scale;
//
//    float parentBoxXEnd = parentTf.position.x +
//        ((parentR.sprite.plot.rect.w * parentTf.scale.x) / 2.0f);
//
//    swordTf.position = {
//        parentBoxXEnd + ((swordR.sprite.plot.rect.w * swordTf.scale.x) / 2.0f),
//        parentTf.position.y
//    };
//
//    auto parentBBox = GetSpriteEntityBoundingBox(parentEnt);
//    auto swordBBox = GetSpriteEntityBoundingBox(parentEnt);
//}


auto SwordHandler::MakeSwordSwingCallback()
{
    return [this](const events::GameControllerInput& ev) 
    {
        if (ev.input.state != InputState::Pressed) { return; }
        if (activeFrame_ != kInactive)             { return; }

        activeFrame_ = 0;
    };
}

SwordHandler::SwordHandler(Entity& parent, EventBus2& bus, SDL_Renderer* renderer, 
                           TextureRepository& textureRepo) :
    coordinator_(ECS::CreateEntity()), bodyParentId_(parent.GetID())
{
    assert(parent.IsValid());
    assert(parent.HasComponent<RigidBody>());
    assert(parent.HasComponent<Collider>());

    auto parentRelations = parent.GetRelations();
    assert(!parentRelations.IsChild());

    auto& parentBody = parent.GetComponent<RigidBody>().body;
    auto& parentCollider = parent.GetComponent<Collider>().shape;
    assert(parentCollider.GetData().IsValid());

    auto parentColliderBbox = parentCollider.GetData().GetBoundingBox();

    // origin position for sword sprite
    float swordSpriteXOrigin = parentColliderBbox.x + parentColliderBbox.w;
    float swordSpriteYOrigin = parentColliderBbox.y;

    for (size_t i = 0; i < swordFrames_.size(); i++)
    {
        auto& child = swordFrames_[i];
        child = parentRelations.AddChild();

        child.AddComponent(Transform{});

        auto& hitboxInfo = kSwordHitboxInfo[i];

        // find origin point for sword sprite
        SDL_FPoint localPosOrigin = {
            swordSpriteXOrigin + hitboxInfo.spriteLocalPos.x,
            swordSpriteYOrigin + hitboxInfo.spriteLocalPos.y,
        };
        // move origin to its center location
        SDL_FPoint localPosCenter = {
            localPosOrigin.x + (hitboxInfo.dimensions.w / 2.0f),
            localPosOrigin.y + (hitboxInfo.dimensions.h / 2.0f)
        };

        child.AddComponent(ComponentBuilder<Collider>{}
        .WithShapeParameters({
            .shapeType = B2Shape::Type::Polygon,
            .dimensions = hitboxInfo.dimensions,
            .localPosition = localPosCenter,
        })
        .WithColliderSettings({
            .enableEvents = { .sensor = true },
            .isSensor = true
        }).Build(parentBody));
    }

    auto& tks = coordinator_.AddComponent<SignalTokenStorage>().signalTokens;
    //tks.emplace_back(bus.ConnectToInput(GameControllerInputSource::X, )
}



Result<Void> SwordHandler::LoadSprites(SDL_Renderer* renderer, TextureRepository& textureRepo)
{
    SpriteDescriptors package{
        .seriesName = "sword_swing"
    };
    package.data = kSwordSpritePaths
        | std::views::transform([](const auto& str) { return ResourcePath::Sprite(str); })
        | std::views::filter([](auto&& res) { return res.Success(); })
        | std::views::transform([](auto&& res) { 
            return SpriteDescriptor{ .filepath = std::move(res.GetValue())  }; 
        }) | std::ranges::to<std::vector>();

    assert(package.data.size() == swordFrames_.size());

    TRY(SpriteAtlas::Create(renderer), temp);
    TRY(temp.LoadSprites(renderer, std::move(package)), spriteSeries);
    TRY(textureRepo.AttachAtlas(std::move(temp)));

    for (size_t i = 0; i < swordFrames_.size(); i++)
    {
        auto& ent = swordFrames_[i];

        ent.AddComponent(SpriteRenderableComponent{
            .sprite = spriteSeries[i]
        });
    }

}

ui::PointDrawHandler::PointDrawHandler(EventBus2& bus)
{ 
    signalTokens_.push_back(bus.ConnectToInput(
        MouseInputSource::LeftButton, GetLayPointCallback()));
    signalTokens_.push_back(bus.ConnectToInput(
        MouseInputSource::RightButton, GetErasePointCallback()));
}

void ui::PointDrawHandler::Draw(SDL_Renderer* renderer)
{
    if (points_.empty()) { return; }

    auto [r, g, b, a] = SDL_Color{};
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

    SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);

    if (points_.size() == 1)
    {
        SDL_RenderDrawPointF(renderer, points_.back().x, points_.back().y);
    }
    else
    {
        SDL_RenderDrawLinesF(renderer, points_.data(), points_.size());
    }

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

Result<Void> ui::TestButton()
{
    auto fixResult = SceneFixture::GetInstance();
    ASSERT_RESULT(fixResult);
    auto& fixture = fixResult.GetValue();

    auto& repo = fixture->GetTextureRepository();

    auto atlasResult = fixture->LoadGlyphAtlas(
        ResourcePath::Font("GoNotoKurrent-Bold.ttf"), 24);
    ASSERT_RESULT(atlasResult);
    auto& glyphAtlas = atlasResult.GetValue();

    auto entity = ECS::CreateEntity();

    auto& tf = entity.AddComponent(Transform{
        //.position = { SDLite::kFWindowCenter.x - 100.0f,
        //              SDLite::kFWindowCenter.y - 100.0f }
        .position = { SDLite::Window().GetLocalCenter<SDL_FPoint>().x - 100.0f,
                      SDLite::Window().GetLocalCenter<SDL_FPoint>().y - 100.0f }
        });

    auto& textRenderable = entity.AddComponent(TextRenderableComponent{
        .writer = glyphAtlas->GetTextWriter(),
        .formatting = {.bounds = { 100, 500 } }
        });

    static constexpr std::string_view kYouPressedIt = "Yay you pressed it!";

    auto wsResult = ui::Workspace::Create(
        fixture->GetRenderer(),
        fixture->GetTextureRepository(),
        fixture->GetEventBus()
    );
    ASSERT_RESULT(wsResult);

    auto& workspace = wsResult.GetValue();

    auto handleResult = workspace.PlaceButton({
        .position = SDLite::kFWindowCenter,
        .color = ui::Button::Color::Green,
        .onClick = [entity]() mutable {
            entity.GetComponent<TextRenderableComponent>().writer.text = kYouPressedIt;
        }
        });

    ASSERT_RESULT(fixture->RunGameLoop());
}
