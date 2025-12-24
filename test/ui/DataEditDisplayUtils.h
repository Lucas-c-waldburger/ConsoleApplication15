#pragma once
#include "../../core/commonObjects.h"
#include "../../systems/GuiSystem.h"
#include <SDL_rect.h>
#include <vector>
#include <array>
#include <format>
#include "../../core/Handle.h"

namespace detail {
template <typename T> struct is_optional : std::false_type {};
template <typename T> struct is_optional<std::optional<T>> : std::true_type{};

template <typename T> struct is_handle : std::false_type {};
template <typename T> struct is_handle<Handle<T>> : std::true_type {};
} // detail

template <typename T> inline constexpr bool is_optional_v = detail::is_optional<T>::value;
template <typename T> inline constexpr bool is_handle_v = detail::is_handle<T>::value;


class SimpleGuiTable
{
public:
    struct Row
    {
        std::string label;
        std::string value;

        template <typename...Ts> requires (sizeof...(Ts) > 0)
        void SetValue(Ts&&...values)
        {
            std::stringstream ss;
            ((ss << std::forward<Ts>(values)), ...);

            value = ss.str();
        }
    };

    SimpleGuiTable(std::string tableName) : tableName_(std::move(tableName))
    {}

    template <typename...Ts> 
        requires (sizeof...(Ts) > 0 && (std::convertible_to<Ts, std::string> && ...))
    void DefineRows(Ts&&...labels)
    {
        rows_.resize(sizeof...(Ts));

        [&]<size_t...Is>(std::index_sequence<Is...>) {
            ((rows_[Is].label = std::forward<Ts>(labels)), ...);
        }(std::make_index_sequence<sizeof...(Ts)>{});
    }

    void Draw()
    {
        if (ImGui::BeginTable(tableName_.c_str(), 2,
            ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
        {
            for (const auto& row : rows_)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(row.label.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(row.value.c_str());
            }

            ImGui::EndTable();
        }
    }

    Row& operator[](size_t i)
    {
        assert(i < rows_.size());

        return rows_[i];
    }

private:
    std::string tableName_;
    std::vector<Row> rows_;
};


//template <FixedString Name, typename T>
//struct draw_custom_data_editor
//{
//    static constexpr FixedString name = Name;
//    using type = T;
//};


//template <typename T>
//concept CustomUiEditType = requires(T& t) {
//    { DrawCustomEditor(t) } -> std::same_as<bool>;
//};

//inline bool DrawCustomEditor(TextureMods& mods)
//{
//    bool changed = false;
//
//    float rgba[4] = {
//        static_cast<float>(mods.color.r),
//        static_cast<float>(mods.color.g),
//        static_cast<float>(mods.color.b),
//        static_cast<float>(mods.alpha)
//    };
//
//    if (ImGui::ColorPicker4("Color", rgba))
//    {
//        mods.color = {
//            .r = static_cast<int>(rgba[0]),
//            .g = static_cast<int>(rgba[1]),
//            .b = static_cast<int>(rgba[2]),
//        };
//        mods.alpha = rgba[3];
//
//        changed = true;
//    }
//
//    static constexpr const char* kBlendModeStrings[] = {
//        "None", "Blend", "Add", "Modulate", "Multiply"
//    };
//
//    static constexpr std::array kBlendModeValues = {
//        SDL_BLENDMODE_NONE,
//        SDL_BLENDMODE_BLEND,
//        SDL_BLENDMODE_ADD,
//        SDL_BLENDMODE_MOD,
//        SDL_BLENDMODE_MUL
//    };
//
//    int currentIdx = [blend = mods.blend] {
//        for (size_t i = 0; i < sizeof(kBlendModeValues); i++) 
//        {
//            if (blend == kBlendModeValues[i]) 
//            {
//                return static_cast<int>(i);
//            }
//        }
//        return 0;
//    }();
//
//    if (ImGui::Combo("Blend Mode", &currentIdx, kBlendModeStrings,
//        IM_ARRAYSIZE(kBlendModeStrings)))
//    {
//        assert(currentIdx >= 0 && currentIdx < kBlendModeValues.size());
//        mods.blend = kBlendModeValues[static_cast<size_t>(currentIdx)];
//
//        changed = true;
//    }
//
//    return changed;
//}



//template <typename T>
//inline T GetDefaultValue()
//{
//    if constexpr (std::is_arithmetic_v<T>)
//    {
//        return static_cast<T>(0);
//    }
//    else if constexpr (std::is_default_constructible_v<T>)
//    {
//        return T{};
//    }
//    else
//    {
//        static_assert(sizeof(T) == 0, "No default value could be generated");
//    }
//}
//
//inline bool DrawEditFields(std::string_view name, auto& member)
//{
//    using Type = std::remove_cvref_t<decltype(member)>;
//
//    std::string id = std::format("##{}", name);
//    bool changed = false;
//
//    if constexpr (std::same_as<Type, bool>)
//    {
//        changed = ImGui::Checkbox(id.c_str(), &member);
//    }
//    else if constexpr (std::is_integral_v<Type>)
//    {
//        if constexpr (sizeof(Type) > sizeof(int))
//        {
//            assert(member <= static_cast<Type>(std::numeric_limits<int>::max()));
//            assert(member >= static_cast<Type>(std::numeric_limits<int>::min()));
//        }
//
//        int i = static_cast<int>(member);
//        if (changed = ImGui::DragInt(id.c_str(), &i))
//        {
//            member = static_cast<Type>(i);
//        }
//    }
//    else if constexpr (std::is_floating_point_v<Type>)
//    {
//        if constexpr (sizeof(Type) > sizeof(int))
//        {
//            assert(member <= static_cast<Type>(std::numeric_limits<float>::max()));
//            assert(member >= static_cast<Type>(std::numeric_limits<float>::min()));
//        }
//
//        float f = static_cast<float>(member);
//        if (changed = ImGui::DragFloat(id.c_str(), &f))
//        {
//            member = static_cast<Type>(f);
//        }
//    }
//    else if constexpr (std::same_as<Type, std::string>)
//    {
//        assert(member.size() < 256);
//
//        char buf[256];
//        std::snprintf(buf, sizeof(buf), "%s", member.c_str());
//
//        if (changed = ImGui::InputText(id.c_str(), buf, sizeof(buf)))
//        {
//            member = buf;
//        }
//    }
//    else if constexpr (is_handle_v<Type>)
//    {
//        ImGui::LabelText(id.c_str(), member.GetHash());
//    }
//    else if constexpr (is_optional_v<Type>)
//    {
//        using ValueType = typename Type::value_type;
//
//        bool hasValue = member.has_value();
//
//        if (ImGui::Checkbox(id.c_str(), &hasValue))
//        {
//            if (hasValue && !member.has_value())
//            {
//                member.emplace(GetDefaultValue<ValueType>());
//                changed = true;
//            }
//            else if (!hasValue && member.has_value())
//            {
//                member.reset();
//                changed = true;
//            }
//        }
//
//        if (member.has_value())
//        {
//            changed |= DrawAggregateEditFieldsImpl(name, *member);
//        }
//    }
//    else if constexpr (std::is_aggregate_v<Type>)
//    {
//        ImGui::TextUnformatted(name.data());
//        ImGui::Indent();
//
//        boost::pfr::for_each_field_with_name(member,
//        [&changed](std::string_view nm, auto& v) {
//            changed |= DrawEditFields(nm, v);
//        });
//
//        ImGui::Unindent();
//    }
//    else
//    {
//        std::string txt = std::format("{}: <unsupported>", id.c_str());
//        ImGui::Text(txt.c_str());
//    }
//
//    return changed; 
//}

//template <typename T>
//inline bool DrawEditFields(std::string_view label, T& val)
//{
//    bool changed = false;
//
//    ImGui::TextUnformatted(label.data());
//    ImGui::Indent();
//
//    boost::pfr::for_each_field_with_name(val,
//    [&changed](std::string_view nm, auto& v) {
//        changed |= DrawAggregateEditFieldsImpl(nm, v);
//    });
//
//    ImGui::Unindent();
//
//    return changed;
//}

//namespace detail {
//template <typename T> struct escape_char;
//
//template <> struct escape_char<int> { static constexpr std::string_view value = "%d"; };
//template <> struct escape_char<float> { static constexpr std::string_view value = "%f"; };
//template <> struct escape_char<std::string> { static constexpr std::string_view value = "%s"; };
//} // detail
//
//template <typename T>
//inline constexpr std::string_view escape_char_v = detail::escape_char<T>::value;

//template <typename T>
//using DragFnSig = bool(*)(const char*, T*, float, T, T, const char*, ImGuiSliderFlags);
//
////template <typename T>
////inline bool DrawValueLabel(std::string_view name, const T& value)
////{
////    return ImGui::LabelText(std::format("{}: {}", name, value));
////}
//
//template <typename T>
//concept AggregateAllSameTypes = pfr::
//
//template <typename T> requires std::is_arithmetic_v<T>
//inline bool EditNumber(std::string_view name, T& v, float vSpeed = 1.0f)
//{
//    if constexpr (std::same_as<T, int>)
//    {
//        changed |= (ImGui::DragInt(name.data(), v, 0.1f));
//    }
//    else
//    {
//        changed |= (ImGui::DragInt2(name.data(), v, 1));
//    }
//}
//
//template <typename T> requires (IntOrFloat<T> || SDLPointType<T>)
//inline bool EditNumeric(std::string_view name, T& val, float vSpeed = 1.0f)
//{
//    using SigArg = std::conditional_t<SDLPointType<T>, decltype(T::x), T>;
//
//    DragFnSig<SigArg> fnSig = nullptr;
//    SigArg* arg = nullptr;
//
//    if constexpr (IntOrFloat<T>)
//    {
//        arg = &val;
//        if constexpr (std::same_as<T, int>)
//        {
//            fnSig = &ImGui::DragInt;
//        }
//        else
//        {
//            fnSig = &ImGui::DragFloat;
//        }
//    }
//    else 
//    {
//        SigArg v[2] = { val.x, val.y };
//        arg = v;
//
//        if constexpr (std::same_as<SigArg, int>)
//        {
//            fnSig = &ImGui::DragInt2;
//        }
//        else
//        {
//            fnSig = &ImGui::DragFloat2;
//        }
//    }
//
//    assert(fnSig);
//
//    bool changed = (*fnSig)(name.data(), arg, vSpeed);
//
//    if constexpr (IntOrFloat<T>)
//    {
//        if constexpr (std::same_as<T, int>)
//        {
//            changed |= (ImGui::DragInt(name.data(), v, 0.1f));
//        }
//        else
//        {
//            changed |= (ImGui::DragInt2(name.data(), v, 1));
//        }
//    }
//    else
//    {
//        using ValueType = decltype(P::x);
//        ValueType v[2] = { p.x, p.y };
//
//
//
//        if constexpr (std::same_as<P, SDL_Point>)
//        {
//            changed |= (ImGui::DragFloat2(name.data(), v, vSpeed));
//        }
//        else
//        {
//            changed |= (ImGui::DragInt2(name.data(), v, vSpeed));
//        }
//
//        if (changed)
//        {
//            p.x = v[0];
//            p.y = v[1];
//        }
//
//        return changed;
//    }
//}

//inline bool EditFloat(const char* label, float& f, float vSpeed = 1.0f, )
//{
//    return ImGui::DragFloat(label, &f);
//}

// RANGE
template <typename T>
inline bool DrawRange(const char* label, Range<T>& range);

template <>
inline bool DrawRange<SDL_FPoint>(const char* label, Range<SDL_FPoint>& range)
{
    bool changed = false;

    ImGui::TextUnformatted(label);
    ImGui::Indent();

    {
        float v[2] = { range.min.x, range.min.y };
        if (ImGui::DragFloat2("min", v, 0.1f))
        {
            range.min.x = v[0];
            range.min.y = v[1];
            changed = true;
        }
    }

    {
        float v[2] = { range.max.x, range.max.y };
        if (ImGui::DragFloat2("max", v, 0.1f))
        {
            range.max.x = v[0];
            range.max.y = v[1];
            changed = true;
        }
    }

    ImGui::Unindent();

    if (range.min.x > range.max.x)
    {
        range.min.x = range.max.x;
        changed = true;
    }
    if (range.min.y > range.max.y)
    {
        range.min.y = range.max.y;
        changed = true;
    }

    return changed;
}

template <>
inline bool DrawRange<float>(const char* label, Range<float>& range)
{
    bool changed = false;

    ImGui::TextUnformatted(label);
    ImGui::Indent();

    if (ImGui::DragFloat("min", &range.min, 0.1f))
    {
        changed = true;
    }
    if (ImGui::DragFloat("max", &range.max, 0.1f))
    {
        changed = true;
    }

    ImGui::Unindent();

    if (range.min > range.max)
    {
        range.min = range.max;
        changed = true;
    }

    return changed;
}

// DIMENSIONS
template <typename T>
inline bool DrawDimensions(const char* label, Dimensions<T>& dims);

template <>
inline bool DrawDimensions<float>(const char* label, Dimensions<float>& dims)
{
    bool changed = false;

    ImGui::TextUnformatted(label);
    ImGui::Indent();

    if (ImGui::DragFloat("w", &dims.w, 0.1f))
    {
        changed = true;
    }
    if (ImGui::DragFloat("h", &dims.h, 0.1f))
    {
        changed = true;
    }

    ImGui::Unindent();

    return changed;
}


// OPTIONAL
template <typename T>
inline bool DrawOptional(const char* label, std::optional<T>& op);

template <>
inline bool DrawOptional<float>(const char* label, std::optional<float>& op)
{
    bool changed = false;
    bool hasValue = op.has_value();

    ImGui::TextUnformatted(label);
    ImGui::Indent();

    if (ImGui::Checkbox(label, &hasValue))
    {
        if (hasValue && !op.has_value())
        {
            op = 0.0f;
            changed = true;
        }
        else if (!hasValue && op.has_value())
        {
            op.reset();
            changed = true;
        }
    }

    if (op.has_value())
    {
        float v = *op;
        if (ImGui::DragFloat("value", &v, 0.1f))
        {
            *op = v;
            changed = true;
        }
    }

    ImGui::Unindent();

    return changed;
}

template <>
inline bool DrawOptional<Dimensions<float>>(const char* label, 
                                            std::optional<Dimensions<float>>& op)
{
    bool changed = false;
    bool hasValue = op.has_value();

    ImGui::TextUnformatted(label);
    ImGui::Indent();

    if (ImGui::Checkbox(label, &hasValue))
    {
        if (hasValue && !op.has_value())
        {
            op = Dimensions<float>{ 0.0f, 0.0f };
            changed = true;
        }
        else if (!hasValue && op.has_value())
        {
            op.reset();
            changed = true;
        }
    }

    if (op.has_value())
    {
        if (ImGui::DragFloat("w", &op->w, 0.1f))
        {
            changed = true;
        }
        if (ImGui::DragFloat("h", &op->h, 0.1f))
        {
            changed = true;
        }
    }

    ImGui::Unindent();

    return changed;
}

template <>
inline bool DrawOptional<SDL_FPoint>(const char* label, std::optional<SDL_FPoint>& op)
{
    bool changed = false;
    bool hasValue = op.has_value();

    ImGui::TextUnformatted(label);
    ImGui::Indent();

    if (ImGui::Checkbox(label, &hasValue))
    {
        if (hasValue && !op.has_value())
        {
            op = { 0.0f, 0.0f };
            changed = true;
        }
        else if (!hasValue && op.has_value())
        {
            op.reset();
            changed = true;
        }
    }

    if (op.has_value())
    {
        float v[2] = { op->x, op->y };
        if (ImGui::DragFloat2("value", v, 0.1f))
        {
            op->x = v[0];
            op->y = v[1];
            changed = true;
        }
    }

    ImGui::Unindent();

    return changed;
}

template <>
inline bool DrawOptional<std::vector<SDL_FPoint>>(const char* label, 
    std::optional<std::vector<SDL_FPoint>>& op)
{
    bool changed = false;
    bool hasValue = op.has_value();

    ImGui::TextUnformatted(label);
    ImGui::Indent();

    if (ImGui::Checkbox(label, &hasValue))
    {
        if (hasValue && !op.has_value())
        {
            op = std::vector<SDL_FPoint>{};
            changed = true;
        }
        else if (!hasValue && op.has_value())
        {
            op.reset();
            changed = true;
        }
    }

    if (!op.has_value())
    {
        return changed;
    }

    auto& vec = *op;

    if (ImGui::Button("Add"))
    {
        vec.emplace_back();
        changed = true;
    }

    for (size_t i = 0; i < vec.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));

        if (ImGui::TreeNode("Item"))
        {
            float v[2] = { vec[i].x, vec[i].y };
            if (ImGui::DragFloat2("value", v, 0.1f))
            {
                vec[i].x = v[0];
                vec[i].y = v[1];
                changed = true;
            }

            if (ImGui::Button("Remove"))
            {
                vec.erase(vec.begin() + i);
                changed = true;
                ImGui::TreePop();
                ImGui::PopID();
                break;
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::Unindent();

    return changed;
}