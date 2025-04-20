#pragma once
#include "BaseComponent.h"
#include "../core/commonObjects.h"
#include <SDL.h>
#include "../core/Logger.h"

//struct Profile 
//{
//    enum : uint8_t
//    {
//        Invalid = 0,
//        Solid = 1 << 0,
//        NonSolid = 1 << 1,
//        Static = 1 << 2,
//        Dynamic = 1 << 3
//    };
//
//    bool 
//
//    uint8_t& operator=(const uint8_t newVal)
//    {
//        if (newVal & (Solid | NonSolid))
//        {
//
//        }
//    }
//
//    uint8_t value = 0;
//};

struct Collider : public BaseComponent<Collider, 8>
{
    struct Material
    {
        float restitution = 0.0f;
        float friction = 0.5f;
    };

    enum Profile : uint8_t
    {
        Solid = 1 << 0,
        NonSolid = 1 << 1,
        Static = 1 << 2,
        Dynamic = 1 << 3,
        ApplyScale = 1 << 4
    };
    static constexpr inline uint8_t kDefaultProfile = (ApplyScale | Solid | Dynamic);

    SDL_FPoint position = { 0.0f, 0.0f };
    Dimensions<float> dimensions = { 0.0f, 0.0f };
    Material material = {};
    uint8_t profile = kDefaultProfile;
};

// TODO: Move this somewhere not stupid
static bool IsColliderProfileValid(const uint8_t profile)
{
    bool atLeastOneSolidNonSolid = ((profile & (Collider::Solid | Collider::NonSolid)) != 0);
    bool onlyOneSolidNonSolid = 
        ((profile & (Collider::Solid | Collider::NonSolid)) != (Collider::Solid | Collider::NonSolid));

    bool atLeastOneStaticDynamic = ((profile & (Collider::Static | Collider::Dynamic)) != 0);
    bool onlyOneStaticDynamic = 
        ((profile & (Collider::Static | Collider::Dynamic)) != (Collider::Static | Collider::Dynamic));

    if (!atLeastOneSolidNonSolid)
    {
        LOG_ERROR("Collider profile did not have Solid or NonSolid selected");
        return false;
    }
    if (!onlyOneSolidNonSolid)
    {
        LOG_ERROR("Collider profile was set to both Solid and NonSolid");
        return false;
    }
    if (!atLeastOneStaticDynamic)
    {
        LOG_ERROR("Collider profile did not have Static or Dynamic selected");
        return false;
    }
    if (!onlyOneStaticDynamic)
    {
        LOG_ERROR("Collider profile was set to both Static and Dynamic");
        return false;
    }

    return true;
}