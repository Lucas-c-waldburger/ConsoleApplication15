#pragma once
#include <string>
#include <vector>

template <typename MetadataType>
class ResourcePacket
{
public:
	void SetFilepaths(const std::vector<std::string>& filepaths);
	void SetFilepaths(std::vector<std::string>&& filepaths);

	void SetMetadata(const MetadataType& metadata);
	void SetMetadata(MetadataType&& metadata);

	const std::vector<std::string>& GetFilepaths() const { return filepaths_; }
	const MetadataType& GetMetadata() const { return metadata_; }

	bool IsValid() const { return validFlags_ & (FilepathsSet | MetadataSet); }

private:
	enum : uint8_t
	{
		FilepathsSet,
		MetadataSet
	};

	std::vector<std::string> filepaths_;
	MetadataType metadata_;
	uint8_t validFlags_ = 0;
};


template <typename T>
using ResourcePackets = std::vector<ResourcePacket<T>>;


template <typename MetadataType>
inline void ResourcePacket<MetadataType>::SetFilepaths(const std::vector<std::string>& filepaths)
{
	filepaths_ = filepaths;
	validFlags_ |= FilepathsSet;
}

template <typename MetadataType>
inline void ResourcePacket<MetadataType>::SetFilepaths(std::vector<std::string>&& filepaths)
{
	filepaths_ = std::move(filepaths);
	validFlags_ |= FilepathsSet;
}

template <typename MetadataType>
inline void ResourcePacket<MetadataType>::SetMetadata(const MetadataType& metadata)
{
	metadata_ = metadata;
	validFlags_ |= MetadataSet;
}

template <typename MetadataType>
inline void ResourcePacket<MetadataType>::SetMetadata(MetadataType&& metadata)
{
	metadata_ = std::move(metadata);
	validFlags_ |= MetadataSet;
}
