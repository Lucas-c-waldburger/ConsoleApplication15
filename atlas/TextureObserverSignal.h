#pragma once
#include "../core/Signal.h"
#include "TextureResourceHandle.h"

class TextureAtlas;
struct SDL_Texture;

class TextureObserverPassKey;

class TextureObserver
{
protected:
    TextureObserverPassKey GetTextureObserverPassKey() const;
};

class TextureObserverPassKey
{
    friend class TextureObserver;
    constexpr TextureObserverPassKey() = default;
    static constexpr TextureObserverPassKey Get() { return {}; }
};

using TextureObserverSignal = 
    PrivateSignal<TextureObserverPassKey, TextureAtlasID, SDL_Texture*>;

class TextureCreationNotifier
{
public:
    template <typename Fn>
    SignalToken ConnectTextureObserver(TextureObserverPassKey pk, Fn&& fn)
    {
        signal_ = {};

        return signal_.Connect(pk, std::forward<Fn>(fn));
    }

protected:
    void NotifyTextureCreated(TextureAtlasID, SDL_Texture* texture);

private:
    TextureObserverSignal signal_;
};