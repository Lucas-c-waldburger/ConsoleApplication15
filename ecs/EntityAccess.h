#pragma once

class EntityPassKey;

class EntityFullAccessPrivelage
{
protected:
    static EntityPassKey GetEntityPassKey(); 
};

class EntityPassKey
{
    friend class EntityFullAccessPrivelage;
    constexpr EntityPassKey() = default;
    static constexpr EntityPassKey Get() { return EntityPassKey{}; }
};



