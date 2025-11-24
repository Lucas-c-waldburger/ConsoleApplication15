#pragma once
#include "../../ecs/Ecs.h"
#include "../../systems/GuiSystem.h"
#include "../../serial/Serialization.h"
#include "../../systems/SerializationSystem.h"


namespace ui {

static constexpr std::string_view kComponentListJsonFilename = "component_list.json";

//Result<Void> MakeComponentListJson()
//{
//	auto e = ECS::CreateEntity();
//
//	SerializableComponentTypeList::ForEachType([&e]<typename T> {
//		e.AddComponent<T>();
//	});
//
//	auto result = SerializationSystem{}.SerializeEntities(kComponentListJsonFilename);
//
//	e.Destroy();
//
//	return result;
//}

struct ComponentEditContext
{
    nlohmann::ordered_json defaultsJson;
    nlohmann::ordered_json runningJson;
    std::string_view selectedComponentForEdit;
    std::string errorMessage = "Unknown error";
    TextureRepository textureRepository;
};

//std::string DrawLabel(std::string_view sv)
//{
//    std::string id = std::format("{}##label", sv);
//    ImGui::Text("%s", sv.data());
//    ImGui::SameLine();
//}

void DrawPrimitiveEditField(nlohmann::ordered_json& j, bool& wasEdited,
                            const std::string& label, bool drawText = true)
{
    using json = nlohmann::json;

    ImGui::Text("%s", drawText ? label.c_str() : " ");
    ImGui::SameLine();

    std::string id = "##" + label;

    switch (j.type())
    {
    case json::value_t::number_float:
    {
        float f = j.get<float>();
        if (ImGui::DragFloat(id.c_str(), &f, 0.1f))
        {
            j = f;
            wasEdited = true;
        }
        break;
    }

    case json::value_t::number_integer:
    {
        int i = j.get<int>();
        if (ImGui::DragInt(id.c_str(), &i))
        {
            j = i;
            wasEdited = true;
        }
        break;
    }

    case json::value_t::number_unsigned:
    {
        unsigned u = j.get<unsigned>();
        int temp = static_cast<int>(u);
        if (ImGui::DragInt(id.c_str(), &temp))
        {
            j = static_cast<unsigned>(temp);
            wasEdited = true;
        }
        break;
    }

    case json::value_t::boolean:
    {
        bool b = j.get<bool>();
        if (ImGui::Checkbox(id.c_str(), &b))
        {
            j = b;
            wasEdited = true;
        }
        break;
    }

    case json::value_t::string:
    {
        std::string s = j.get<std::string>();
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s", s.c_str());

        if (ImGui::InputText(id.c_str(), buf, sizeof(buf)))
        {
            j = std::string(buf);
            wasEdited = true;
        }
        break;
    }

    default:
        ImGui::Text("%s: <unsupported>", label.c_str());
        break;
    }
}

bool IsFlatObject(const nlohmann::json& j)
{
    if (!j.is_object())
    {
        return true;
    }

    for (auto& [key, value] : j.items())
    {
        if (value.is_object() || value.is_array())
        {
            return false;
        }
    }

    return true;
}

void DrawFlatObject(nlohmann::ordered_json& j, bool& wasEdited)
{
    auto items = j.items();

    for (auto& [key, value] : items)
    {
        DrawPrimitiveEditField(value, wasEdited, key, true);
    }
}

void DrawJsonSuppliedEditField(nlohmann::ordered_json& j, bool& wasEdited, 
                               const std::string& label, bool drawText = true)
{
    using json = nlohmann::json;

    ImGui::PushID(label.c_str());

    ImGuiTreeNodeFlags treeNodeFlags = (IsFlatObject(j)) 
        ? ImGuiTreeNodeFlags_DefaultOpen
        : 0;

    if (ImGui::TreeNodeEx(label.c_str(), treeNodeFlags))
    {
        ImGui::Indent();

        switch (j.type())
        {
        case json::value_t::object:
            if (IsFlatObject(j))
            {
                DrawFlatObject(j, wasEdited);
                break;
            }

            for (auto& [key, value] : j.items())
            {                
                DrawJsonSuppliedEditField(value, wasEdited, 
                                          key, value.size() > 1);
            }
            break;

        case json::value_t::array:
            for (size_t i = 0; i < j.size(); ++i)
            {
                DrawJsonSuppliedEditField(j[i], wasEdited, std::to_string(i));
            }
            break;

        default:
            DrawPrimitiveEditField(j, wasEdited, label, drawText);
            break;
        }

        ImGui::Unindent();
        ImGui::TreePop();
    }

    ImGui::PopID();    
}

template <JsonSerializableComponent T>
void DrawComponentEditor(Entity& e, nlohmann::ordered_json& runningJson)
{
    assert(e.HasComponent<T>());

    constexpr std::string_view componentName = ComponentName<T>::value;

    if (!runningJson.contains(componentName))
    {
        return;
    }

    auto& componentJson = runningJson.at(componentName);
    bool wasEdited = false;

    DrawJsonSuppliedEditField(componentJson, wasEdited, componentName.data());

    if (wasEdited)
    {
        from_json(componentJson, e.GetComponent<T>());
    }
}

template <JsonSerializableComponent T>
void LoadComponent(Entity& e, ComponentEditContext& ctx)
{
    constexpr std::string_view componentName = ComponentName<T>::value;

    assert(ctx.defaultsJson.contains(componentName));

    e.AddComponent<T>();
    ctx.runningJson[componentName] = ctx.defaultsJson[componentName];
}

void DrawExistingComponentList(Entity& e, ComponentEditContext& ctx)
{
    ImGui::Text("Components:");

    SerializableComponentTypeList::ForEachType([&]<typename T>{
        if (e.HasComponent<T>())
        {
            constexpr std::string_view componentName = ComponentName<T>::value;

            bool selected = (ctx.selectedComponentForEdit == componentName);

            if (ImGui::Selectable(componentName.data(), selected))
            {
                ctx.selectedComponentForEdit = componentName;
            }
        }
    });
}

void DrawAddComponentList(Entity& e, ComponentEditContext& ctx)
{
    SerializableComponentTypeList::ForEachType([&]<typename T>{
        if (e.HasComponent<T>())
        {
            return;
        }

        constexpr std::string_view componentName = ComponentName<T>::value;

        if (ImGui::Selectable(componentName.data()))
        {
            LoadComponent<T>(e, ctx);
            ImGui::CloseCurrentPopup();
        }
    });
}

class EntityInspector
{
public:
    static bool Draw(Entity& e)
    {
        bool ret = true;

        //ImGui::Begin("Entity Inspector");

        if (ImGui::BeginPopupModal("ErrorPopup", nullptr, 
                                    ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextWrapped("%s", editContext_.errorMessage.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();

            ret = false;
        }

        if (editContext_.defaultsJson.empty())
        {
            auto loadResult = LoadComponentDefaultsJson();
            if (!loadResult.Success())
            {
                editContext_.errorMessage = loadResult.GetError().GetMessage();
                ImGui::OpenPopup("ErrorPopup");
            }
        }

        if (ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            DrawAddComponentList(e, editContext_);

            ImGui::EndPopup();
        }

        DrawExistingComponentList(e, editContext_);

        // Component editor
        if (!editContext_.selectedComponentForEdit.empty())
        {
            ImGui::Separator();
            ImGui::Text("Editing: %s", editContext_.selectedComponentForEdit.data());

            SerializableComponentTypeList::ForEachType([&]<typename T>{
                if (!e.HasComponent<T>())
                {
                    return;
                }

                if (editContext_.selectedComponentForEdit == ComponentName<T>::value)
                {
                    DrawComponentEditor<T>(e, editContext_.runningJson);
                }
            });
        }

        //ImGui::End();
        
        return ret;
    }


private:
    static Result<Void> LoadComponentDefaultsJson()
    {
        TRY(ResourcePath::Json(kComponentListJsonFilename), path);

        std::ifstream file(path);
        if (!file)
        {
            return MAKE_ERROR_FMT("Could not open exisiting JSON "
                " file at path: '{}'", path);
        }

        nlohmann::ordered_json j;
        try
        {
            j = nlohmann::json::parse(file);
        }
        catch (const nlohmann::json::parse_error& err)
        {
            return MAKE_ERROR_FMT("JSON parse error: '{}'", err.what());
        }

        if (!j.contains("components"))
        {
            return MAKE_ERROR("Components JSON key not found");
        }

        editContext_.defaultsJson = std::move(j.at("components"));
        if (editContext_.defaultsJson.empty())
        {
            return MAKE_ERROR("Components default json was empty");
        }

        return Void{};
    }

    static inline ComponentEditContext editContext_;
};


} // ui
