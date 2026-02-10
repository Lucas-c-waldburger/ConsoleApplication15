#include "NewTextureRepository.h"

TextureRepository::TextureRepository() : tokens_{
	.spriteTextureCreated{
		spriteAtlasCollection_.ConnectTextureObserver(
			GetTextureObserverPassKey(), GetTextureCreatedCallback())},
	.glyphTextureCreated{
		glyphAtlasCollection_.ConnectTextureObserver(
			GetTextureObserverPassKey(), GetTextureCreatedCallback())} }
{}

TextureRepository::~TextureRepository()
{
	tokens_.spriteTextureCreated.Disconnect();
	tokens_.glyphTextureCreated.Disconnect();
}

SDL_Texture* 
TextureRepository::GetSourceTexture(const Handle<TextureResource>& handle) const
{
	auto it = sourceTextureMap_.find(handle.GetAtlasID());

	return (it != sourceTextureMap_.end()) ? it->second : nullptr;
}

Result<Void> TextureRepository::RebuildSourceTextures(SDL_Renderer* renderer)
{
	TRY(spriteAtlasCollection_.RebuildSourceTextures(renderer));
	//TRY(glyphAtlasCollection_.RebuildSourceTextures(renderer));
	return kVoid;
}