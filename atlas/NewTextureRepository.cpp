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
	tokens_.spriteTextureCreated.Reset();
	tokens_.glyphTextureCreated.Reset();
}

TextureRepository::TextureRepository(TextureRepository&& other) noexcept :
	spriteAtlasCollection_(std::move(other.spriteAtlasCollection_)),
	glyphAtlasCollection_(std::move(other.glyphAtlasCollection_)),
	sourceTextureMap_(std::move(other.sourceTextureMap_)),
	tokens_{
		.spriteTextureCreated{
			spriteAtlasCollection_.ConnectTextureObserver(
				GetTextureObserverPassKey(), GetTextureCreatedCallback())},
		.glyphTextureCreated{
			glyphAtlasCollection_.ConnectTextureObserver(
				GetTextureObserverPassKey(), GetTextureCreatedCallback())} }

{}

TextureRepository& TextureRepository::operator=(TextureRepository&& other) noexcept
{
	if (this == &other) { return *this; }

	tokens_ = {};

	spriteAtlasCollection_ = std::move(other.spriteAtlasCollection_);
	glyphAtlasCollection_ = std::move(other.glyphAtlasCollection_);

	sourceTextureMap_ = std::move(other.sourceTextureMap_);

	tokens_.spriteTextureCreated = spriteAtlasCollection_.ConnectTextureObserver(
		GetTextureObserverPassKey(), GetTextureCreatedCallback());
	tokens_.glyphTextureCreated = glyphAtlasCollection_.ConnectTextureObserver(
		GetTextureObserverPassKey(), GetTextureCreatedCallback());

	return *this;
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