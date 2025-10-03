#pragma once

class EntityPassKey;

class EntityFullAccessPrivelage
{
protected:
    EntityPassKey GetEntityPassKey() const; 
};

class EntityPassKey
{
    friend class EntityFullAccessPrivelage;
    constexpr EntityPassKey() = default;
    static constexpr EntityPassKey Get() { return EntityPassKey{}; }
};



