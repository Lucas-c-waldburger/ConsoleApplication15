#include "TextureObserverSignal.h"
#include <SDL_render.h>

TextureObserverPassKey TextureObserver::GetTextureObserverPassKey() const
{
    return TextureObserverPassKey::Get();
}

void TextureCreationNotifier::NotifyTextureCreated(TextureAtlasID atlasId, 
                                                   SDL_Texture* texture)
{
    signal_.Emit(atlasId, texture);
}
