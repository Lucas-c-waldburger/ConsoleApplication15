#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "../GuiResource.h"
#include "../PropertyEditState.h"

namespace ui {

inline int gPropertyDepth = 0;

inline void PushPropertyDepth()
{
    ++gPropertyDepth;
}

inline void PopPropertyDepth()
{
    if (gPropertyDepth > 0) { --gPropertyDepth; }
}

struct PropertyTableIds
{
	static constexpr std::string_view kPropertiesFmt = "Properties_{}";
	static constexpr std::string_view kLabelFmt = "Label_{}";
	static constexpr std::string_view kValueFmt = "Value_{}";

    std::string tableId;
	std::string columnLabelId;
	std::string columnValueId;

    static PropertyTableIds Get(int id)
    {
        return {
            std::format(kPropertiesFmt, id),
            std::format(kLabelFmt, id),
            std::format(kValueFmt, id)
        };
	}
};

template <typename Fn> requires std::same_as<std::invoke_result_t<Fn>, void>
void WithFont(ImFont* font, Fn&& fn)
{
    assert(font);

    ImGui::PushFont(font, font->LegacySize);

    std::invoke(fn);

    ImGui::PopFont();
}  

template <typename Fn> requires std::same_as<std::invoke_result_t<Fn>, bool>
bool WithFont(ImFont* font, Fn&& fn)
{
    assert(font);

    ImGui::PushFont(font, font->LegacySize);

    const auto state = std::invoke(fn);
     
    ImGui::PopFont();

    return state;
}

inline bool BeginPropertyTable(int uniqueId = 0)
{
    gPropertyDepth = 0;
	auto ids = PropertyTableIds::Get(uniqueId);

    if (!ImGui::BeginTable(ids.tableId.c_str(), 2,
        ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_NoBordersInBody))
    {
        return false; 
    }

    ImGui::TableSetupColumn(ids.columnLabelId.c_str(), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(ids.columnValueId.c_str(), ImGuiTableColumnFlags_WidthStretch);

    return true;
}

inline void EndPropertyTable()
{
    ImGui::EndTable();
}

template <typename Fn> requires std::is_invocable_r_v<PropertyEditState, Fn>
PropertyEditState Property(std::string_view label, Fn&& draw)
{
    ImGui::TableNextRow();

    ImGui::TableNextColumn();

    ImGui::AlignTextToFramePadding();

    if (gPropertyDepth > 0)
    {
        ImGui::Indent(gPropertyDepth * 12.0f);
    }

    if (!label.empty())
    {
        ImGui::TextUnformatted(label.data());
    }

    if (gPropertyDepth > 0)
    {
        ImGui::Unindent(gPropertyDepth * 12.0f);
    }

    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(-FLT_MIN);

    return std::invoke(draw);
}

template <typename T, typename...Args>
PropertyEditState Property(std::string_view label, T& value, Args&&...args)
{
    return Property(label, [&]{ 
        ImGui::PushID(&value); 

        const auto state = GuiEditProperty(value, std::forward<Args>(args)...); 

        ImGui::PopID();

        return state;
    });
}

template <typename T, typename...Args>
PropertyEditState Property(std::string_view label, const T& value, Args&&...args)
{
    return Property(label, [&]{
        ImGui::PushID(&value);

        const auto state = GuiDrawProperty(value, std::forward<Args>(args)...);

        ImGui::PopID();

        return state;
    });
}

//template <typename Fn>
//inline bool InvokePropFn(Fn&& fn)
//{
//    bool changed = false;
//    if constexpr (std::same_as<std::invoke_result_t<Fn>, bool>)
//    {
//        changed = std::invoke(fn);
//    }
//    else
//    {
//        std::invoke(fn);
//    }
//
//    return changed;
//}
//
//bool PropertyNextRow(auto col0Fn, auto col1Fn)
//{
//    ImGui::TableNextRow();
//
//    ImGui::TableNextColumn();
//
//    ImGui::AlignTextToFramePadding();
//
//    if (gPropertyDepth > 0)
//    {
//        ImGui::Indent(gPropertyDepth * 12.0f);
//    }
//
//    bool changed = InvokePropFn(col0Fn);
//    //{
//    //    ImGui::TextUnformatted(label.data());
//    //}
//
//    if (gPropertyDepth > 0)
//    {
//        ImGui::Unindent(gPropertyDepth * 12.0f);
//    }
//
//    ImGui::TableNextColumn();
//
//    ImGui::SetNextItemWidth(-FLT_MIN);
//
//    changed |= InvokePropFn(col1Fn);
//
//    return changed;
//}

//template <typename T, typename...Args>
//bool Property(T& value, Args&&...args)
//{
//    return Property("", [&] { return GuiEditProperty(value, std::forward<Args>(args)...); });
//}

//template <typename Fn, typename T>
//bool GuiEditErasableProperty(Fn&& fn, T& val)
//{
//    ImGui::TableNextRow();
//
//    ImGui::TableNextColumn();
//
//    ImGui::AlignTextToFramePadding();
//
//    if (gPropertyDepth > 0)
//    {
//        ImGui::Indent(gPropertyDepth * 12.0f);
//    }
//
//    GuiEditProperty
//
//    if (gPropertyDepth > 0)
//    {
//        ImGui::Unindent(gPropertyDepth * 12.0f);
//    }
//
//    ImGui::TableNextColumn();
//
//    ImGui::SetNextItemWidth(-FLT_MIN);
//
//    const bool changed = std::invoke(draw);
//
//    return changed;
//}

//template <typename Fn> requires std::is_invocable_r_v<bool, Fn>
//bool Component(std::string_view name, Fn&& draw)
//{
//    if (!ImGui::CollapsingHeader(name.data(), ImGuiTreeNodeFlags_SpanFullWidth))
//    {
//        return false;
//    }
//
//    if (BeginPropertyTable())
//    {
//        const bool changed = std::invoke(draw); 
//
//        EndPropertyTable();
//
//        return changed;
//    }
//
//	return false;
//}

struct PropertyGroupOptions
{
    enum class Force : uint8_t
    {
        None,
        Open,
        Close
    };

    Force force = Force::None;
    ImGuiTreeNodeFlags flags = 0;
};

template <typename Fn> requires std::is_invocable_r_v<PropertyEditState, Fn>
PropertyEditState PropertyGroup(std::string_view name, Fn&& draw, PropertyGroupOptions options = {})
{
    ImGui::TableNextRow();

    ImGui::TableNextColumn();

    const ImGuiTreeNodeFlags treeNodeFlags = 
        ImGuiTreeNodeFlags_DrawLinesFull | options.flags;

    const bool open = WithFont(GuiResource::Fonts().semiBold, [&] {

        if (options.force != PropertyGroupOptions::Force::None)
        {
            ImGui::SetNextItemOpen(
                options.force == PropertyGroupOptions::Force::Open, ImGuiCond_Always);
        }

        return ImGui::TreeNodeEx(name.data(), treeNodeFlags);
	});

    ImGui::TableNextColumn();

    if (!open)
    {
        return PropertyEditState::None;
    }

    ++gPropertyDepth;

    ImGui::PushID(name.data());

    const auto state = std::invoke(draw);

    ImGui::PopID();

    --gPropertyDepth;

    ImGui::TreePop();

    return state;
}

template <typename...Fns> 
    requires (sizeof...(Fns) > 0 && (std::same_as<std::invoke_result_t<Fns>, PropertyEditState> && ...))
inline PropertyEditState SplitLine(Fns&&...fns)
{
    static constexpr size_t numFns = sizeof...(fns);

    const float avail = ImGui::GetContentRegionAvail().x;
    const float colW = avail * (1.0f / static_cast<float>(numFns));
    size_t cnt = 1;

    auto impl = [&](auto&& fn) {
        ImGui::BeginGroup();

        if (cnt >= numFns)
        {
            ImGui::PushItemWidth(-FLT_MIN);
        }
        else
        {
            ImGui::PushItemWidth(colW);
        }

        const auto st = std::invoke(fn);
        ImGui::EndGroup();

        if (cnt < numFns)
        {
            ImGui::SameLine();
        }

        ++cnt;

        return st;
    };

    PropertyEditState state = PropertyEditState::None();
    ((state |= impl(fns)), ...);

    return state;
}

//template <typename Fn> requires std::is_invocable_r_v<bool, Fn>
//bool InnerPropertyGroup(std::string_view label, Fn&& draw)
//{
//    ImGui::TableNextRow();
//
//    ImGui::TableNextColumn();
//
//    auto* fnt = GuiResource::Fonts().semiBold;
//    assert(fnt);
//
//    const bool open = WithFont(GuiResource::Fonts().semiBold, [&label] {
//        return ImGui::TreeNodeEx(label.data(), (
//            ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen | 
//            ImGuiTreeNodeFlags_NoAutoOpenOnLog));
//	});
//
//    ImGui::TableNextColumn();
//
//    if (!open)
//    {
//        return false;
//    }
//
//    ++gPropertyDepth;
//
//    ImGui::PushID(label.data());
//
//    const bool changed = std::invoke(draw);
//
//    ImGui::PopID();
//
//    --gPropertyDepth;
//
//    return changed;
//}

} // ui

#endif