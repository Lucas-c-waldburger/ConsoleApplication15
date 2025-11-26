#pragma once
//#include <string>
//#include <vector>
//#include "../core/SOAInterface.h"


//struct SpriteInfo : SOAInterface<SpriteInfo>
//{
//    struct Slice
//    {
//        std::string spriteName;
//        std::string filepath;
//        std::string seriesName;
//        size_t seriesIndex;
//    };
//
//    std::vector<std::string> spriteName;
//    std::vector<std::string> filepath;
//    std::vector<std::string> seriesName;
//    std::vector<size_t> seriesIndex;
//
//    size_t Size() const { return spriteName.size(); }
//
//    size_t PushBack(Slice&& slice)
//    {
//        const size_t newIdx = Size();
//
//        spriteName.emplace_back(std::move(slice.spriteName));
//        filepath.emplace_back(std::move(slice.filepath));
//        seriesName.emplace_back(std::move(slice.seriesName));
//        seriesIndex.emplace_back(std::move(slice.seriesIndex));
//
//        return newIdx;
//    }
//
//    void Reserve(size_t sz)
//    {
//        spriteName.reserve(sz);
//        filepath.reserve(sz);
//        seriesName.reserve(sz);
//        seriesIndex.reserve(sz);
//    }
//
//    Slice MakeSlice(size_t pos) const
//    {
//        if (pos >= Size())
//        {
//            return {};
//        }
//
//        return Slice{
//            .spriteName = spriteName[pos],
//            .filepath = filepath[pos],
//            .seriesName = seriesName[pos],
//            .seriesIndex = seriesIndex[pos]
//        };
//    }
//};


//template <template <typename> class Wrap, typename StrT>
//struct SpriteInfoTemplate
//{
//    //template <typename T> using Wrap = Wrapped<T>;
//    using StrWrap = Wrap<StrT>;
//    using IdxWrap = Wrap<size_t>;
//
//    StrWrap spriteName;
//    StrWrap filepath;
//    StrWrap seriesName;
//    IdxWrap seriesIndex;
//};
//
//struct SpriteInfoView : SpriteInfoTemplate<std::type_identity_t, std::string_view> 
//{};
//struct SpriteInfoRef : SpriteInfoTemplate<std::add_lvalue_reference_t, std::string>
//{};
//struct SpriteInfoCRef : SpriteInfoTemplate<add_const_ref_t, std::string>
//{};


//std::vector<std::string> spriteName;
//std::vector<std::string> filepath;
//std::vector<std::string> seriesName;
//std::vector<size_t> seriesIndex;

//template <typename RefT, typename Self>
//static RefT MakeRef(Self self, size_t pos)
//{
//    assert(pos < self.Size());

//    // Build and return RefT from the exact element types in `self`.
//    // If RefT contains reference members (e.g. T const&), these
//    // initializers will bind to the elements of the vectors.
//    return RefT{
//        static_cast<typename RefT::StrWrap>(self.spriteName[pos]),
//        static_cast<typename RefT::StrWrap>(self.filepath[pos]),
//        static_cast<typename RefT::StrWrap>(self.seriesName[pos]),
//        static_cast<typename RefT::IdxWrap>(self.seriesIndex[pos])
//    };
//}


//SpriteInfoView GetView(size_t pos) const
//{
//    assert(pos < Size());
//    return SpriteInfoView{
//        spriteName[pos],
//        filepath[pos],
//        seriesName[pos],
//        seriesIndex[pos]
//    };
//}


//SpriteInfoRef operator[](size_t pos)
//{
//    assert(pos < Size());
//    return MakeRef<SpriteInfoRef, SpriteInfoSOA&>(*this, pos);
//}

//SpriteInfoCRef operator[](size_t pos) const
//{
//    assert(pos < Size());
//    return MakeRef<SpriteInfoCRef, const SpriteInfoSOA&>(*this, pos);
//}

//template <auto...MemberPtrs>
//auto ForEach()
//{
//    return std::views::zip((*this).*MemberPtrs...);
//}