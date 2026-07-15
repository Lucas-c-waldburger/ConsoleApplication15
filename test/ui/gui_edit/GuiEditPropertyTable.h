#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "../GuiResource.h"

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

    const bool changed = std::invoke(fn);
     
    ImGui::PopFont();

    return changed;
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

template <typename Fn> requires std::is_invocable_r_v<bool, Fn>
bool Property(std::string_view label, Fn&& draw)
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

    const bool changed = std::invoke(draw);

    return changed;
}

template <typename T, typename...Args>
bool Property(std::string_view label, T& value, Args&&...args)
{
    return Property(label, [&]{ 
        ImGui::PushID(&value); 

        const bool changed = GuiEditProperty(value, std::forward<Args>(args)...); 

        ImGui::PopID();

        return changed;
    });
}

template <typename T, typename...Args>
bool Property(std::string_view label, const T& value, Args&&...args)
{
    return Property(label, [&]{
        ImGui::PushID(&value);

        GuiDrawProperty(value, std::forward<Args>(args)...);

        ImGui::PopID();

        return false;
    });
}

template <typename Fn>
inline bool InvokePropFn(Fn&& fn)
{
    bool changed = false;
    if constexpr (std::same_as<std::invoke_result_t<Fn>, bool>)
    {
        changed = std::invoke(fn);
    }
    else
    {
        std::invoke(fn);
    }

    return changed;
}

bool PropertyNextRow(auto col0Fn, auto col1Fn)
{
    ImGui::TableNextRow();

    ImGui::TableNextColumn();

    ImGui::AlignTextToFramePadding();

    if (gPropertyDepth > 0)
    {
        ImGui::Indent(gPropertyDepth * 12.0f);
    }

    bool changed = InvokePropFn(col0Fn);
    //{
    //    ImGui::TextUnformatted(label.data());
    //}

    if (gPropertyDepth > 0)
    {
        ImGui::Unindent(gPropertyDepth * 12.0f);
    }

    ImGui::TableNextColumn();

    ImGui::SetNextItemWidth(-FLT_MIN);

    changed |= InvokePropFn(col1Fn);

    return changed;
}

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

template <typename Fn> requires std::is_invocable_r_v<bool, Fn>
bool Component(std::string_view name, Fn&& draw)
{
    if (!ImGui::CollapsingHeader(name.data(), ImGuiTreeNodeFlags_SpanFullWidth))
    {
        return false;
    }

    if (BeginPropertyTable())
    {
        const bool changed = std::invoke(draw); 

        EndPropertyTable();

        return changed;
    }

	return false;
}

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

template <typename Fn> requires std::is_invocable_r_v<bool, Fn>
bool PropertyGroup(std::string_view name, Fn&& draw, PropertyGroupOptions options = {})
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
        return false;
    }

    ++gPropertyDepth;

    ImGui::PushID(name.data());

    std::invoke(draw);

    ImGui::PopID();

    --gPropertyDepth;

    ImGui::TreePop();

    return true;
}

template <typename Fn> requires std::is_invocable_r_v<bool, Fn>
bool InnerPropertyGroup(std::string_view label, Fn&& draw)
{
    ImGui::TableNextRow();

    ImGui::TableNextColumn();

    auto* fnt = GuiResource::Fonts().semiBold;
    assert(fnt);

    const bool open = WithFont(GuiResource::Fonts().semiBold, [&label] {
        return ImGui::TreeNodeEx(label.data(), (
            ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen | 
            ImGuiTreeNodeFlags_NoAutoOpenOnLog));
	});

    ImGui::TableNextColumn();

    if (!open)
    {
        return false;
    }

    ++gPropertyDepth;

    ImGui::PushID(label.data());

    const bool changed = std::invoke(draw);

    ImGui::PopID();

    --gPropertyDepth;

    return changed;
}

} // ui

#endif