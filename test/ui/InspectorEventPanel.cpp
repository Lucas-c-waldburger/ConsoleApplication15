#include "InspectorEventPanel.h"

#if IMGUI_ENABLED
#include <ranges>
#include <string>
#include <format>
#include "../../events/EventBus2.h"
#include "GuiTexture.h"
#include "gui_edit/GuiEditPropertyTable.h"
#include "../../components/util/ComponentValidPreds.h"
#include "../../core/Algorithms.h"
#include "gui_edit/GuiEditEvents.h"

namespace ui {

constexpr ImVec4 kFiredHeaderColor = ImVec4(0.90f, 0.60f, 0.10f, 1.0f);

using FiredList = TypeIndexedBitset<InspectorEventPanel::GuiEventTypeList>;
using EventList = InspectorEventPanel::GuiEventTypeList::AsTuple<std::type_identity_t>;

bool BeginEventTable()
{
	if (!ImGui::BeginTable("Event Table", 2,
		ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_NoBordersInBody))
	{
		return false;
	}

	ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
	ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

	return true;
}

void EndEventTable()
{
	ImGui::EndTable();
}

template <typename> struct init_event_fired_list_callbacks;

template <template <typename...> class TList, typename...Ts>
struct init_event_fired_list_callbacks<TList<Ts...>>
{
	static void call(EventBus& bus, FiredList& firedList, SignalTokenStorage& tks)
	{
		static constexpr auto impl = []<typename T>(EventBus& bus, FiredList& firedList, 
													SignalTokenStorage& tks) 
		{
			tks.signalTokens.emplace_back(bus.ConnectToEvent([&firedList](const T&) {
				firedList.Set<T>(true);
			}));
		};

		((impl.template operator()<Ts>(bus, firedList, tks)), ...);
	}
};

template <typename T>
bool DrawFireButton(const GuiTextureConverter& converter, bool fired)
{
	auto& button = InspectorEventPanel::GetButtons().fire;
	const auto& sprite = fired ? button.activatedSprite : button.defaultSprite;

	GuiTexture texture{ converter.FromSprite(sprite) };
	auto h = ImGui::GetFrameHeight();
	texture.size.x = h * 1.05f;
	texture.size.y = h * 1.05f;

	assert(texture.textureId != 0);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = (button.isHovered.Test<T>() || fired)
		? ImVec4(1, 1, 1, 1) 
		: ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton(GuiEventName<T>::label, texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	button.isHovered.Set<T>(ImGui::IsItemHovered());

	return pressed;
}

template <typename T>
struct draw_event
{
	static PropertyEditState call(T& ev) 
	{
		PushPropertyDepth();

		const auto state = Property("", ev);

		PopPropertyDepth();

		return state;
	}
};

std::string_view GetEntityName(Entity_t entityId)
{
	if (entityId == kInvalidEntity)
	{
		return {};
	}

	auto e = ECS::GetEntityByID(entityId);
	if (!e.IsValid())
	{
		return {};
	}

	if (!e.HasComponent<Name>())
	{
		return {};
	}

	return e.GetComponent<Name>().value;
}

//struct CollisionDataShapeInfo
//{
//	struct Elem
//	{
//		std::string label;
//		Entity_t entityId = kInvalidEntity;
//		Handle<B2Shape> handle;
//	};
//
//	Elem current;
//	std::vector<Elem> all;
//};

//std::vector<Entity> GetAllColliderEntities(Entity& e)
//{
//	const auto& body = e.GetComponent<RigidBody>().body.GetData();
//	std::vector<Entity> colliderEs;
//
//	if (e.HasComponent<Collider>(&ColliderValid))
//	{
//		colliderEs.emplace_back(e);
//	}
//
//	auto rels = e.GetRelations();
//	if (rels.HasChildren())
//	{
//		auto chs = rels.GetAllChildrenWith<Collider>();
//		for (const auto& ch : chs)
//		{
//			if (ch.HasComponent<Collider>() &&
//				ch.GetComponent<Collider>().shape.GetData().GetParentBodyHandle() ==
//				body.GetHandle())
//			{
//				colliderEs.emplace_back(ch);
//			}
//		}
//	}
//
//	//std::sort(colliderEs.begin(), colliderEs.end(), [](const auto& a, const auto& b) {
//	//	return a.GetID() < b.GetID();
//	//});
//
//	return colliderEs;
//}

std::vector<Entity> GetRigidBodyEntities()
{
	return ECS::GetAllEntitiesWith<Name, RigidBody, Exclude<InspectorTag>>()
	| std::views::filter([](const Entity& e) {
		return e.GetComponent<RigidBody>().body.GetData().IsValid();
	}) | std::ranges::to<std::vector>();
}

//bool IsValidCollisionParticipant(const Entity& e)
//{
//	return e.IsValid() && e.HasComponent<Name>() && e.HasComponent<RigidBody>(&RigidBodyValid);
//}

//CollisionDataShapeInfo GetCollisionDataShapeInfo(Entity& e, const CollisionData& data)
//{
//	if (!IsValidCollisionParticipant(e))
//	{
//		return {};
//	}
//
//	auto colEs = GetAllColliderEntities(e);
//
//	std::sort(colEs.begin(), colEs.end(), [](const Entity& a, const Entity& b) {
//		const auto shA = a.GetComponent<Collider>().shape.GetData();
//		const auto shB = b.GetComponent<Collider>().shape.GetData();
//		if (shA.GetShapeType() == shB.GetShapeType())
//		{
//			return shA.GetHandle() < shB.GetHandle();
//		}
//		return shA.GetShapeType() < shB.GetShapeType();
//	});
//
//	CollisionDataShapeInfo info{};
//	info.all.reserve(colEs.size());
//	
//	size_t counter = 0;
//	B2Shape::Type lastType = B2Shape::Type::Invalid;
//	for (const auto& colE : colEs)
//	{
//		const auto& sh = colE.GetComponent<Collider>().shape.GetData();
//
//		if (sh.GetShapeType() != lastType)
//		{
//			counter = 0;
//		}
//	
//		auto& elem = info.all.emplace_back(
//			std::format("{} {}", ToString(sh.GetShapeType()), counter), 
//			colE.GetID(),
//			sh.GetHandle()
//		);
//	
//		if (data.shapeHandle == elem.handle)
//		{
//			assert(data.entity == elem.entityId);
//			info.current = elem;
//		}
//	
//		++counter;
//	}
//	
//	return info;
//}

//template <typename Ev>
//PropertyEditState DrawCollisionEvent(Ev& ev)
//{
//	static constexpr FixedString kParticipantALabel = "participant A";
//	static constexpr FixedString kParticipantBLabel = "participant B";
//	static constexpr FixedString kShapeALabel = "shape##A";
//	static constexpr FixedString kShapeBLabel = "shape##B";
//
//	const float participantALabelWidth = GetFieldValueWidth<kParticipantALabel, kShapeALabel>();
//	const float participantBLabelWidth = GetFieldValueWidth<kParticipantBLabel, kShapeBLabel>();
//
//	Entity participantA = ECS::GetEntityByID(ev.entity<0>());
//	Entity participantB = ECS::GetEntityByID(ev.entity<1>());
//
//	std::string curNameA;
//	std::string curNameB;
//
//	if (IsValidCollisionParticipant(participantA))
//	{
//		curNameA = participantA.GetComponent<Name>();
//	}
//	if (IsValidCollisionParticipant(participantB))
//	{
//		curNameB = participantB.GetComponent<Name>();
//	}
//
//	auto es = GetRigidBodyEntities();
//
//	ImGui::TextUnformatted(kParticipantALabel);
//	ImGui::SameLine();
//	ImGui::SetNextItemWidth(participantALabelWidth);
//
//	// PARTICIPANT A
//	if (ImGui::BeginCombo("participant A", curNameA.c_str()))
//	{
//		for (const auto& e : es)
//		{
//			assert(e.HasComponent<Name>());
//			const auto& name = e.GetComponent<Name>().value;
//
//			if (name == curNameB)
//			{
//				continue;
//			}
//
//			const bool selected = (name == curNameA);
//			if (ImGui::Selectable(name.c_str(), &selected))
//			{
//				ev.entity<0>() = e.GetID();
//				participantA = e;
//				curNameA = name;
//			}
//		}
//
//		ImGui::EndCombo();
//	}
//
//	ImGui::TableNextColumn();
//	//ImGui::SetNextItemWidth(-FLT_MIN);
//
//	auto aInfo = GetCollisionDataShapeInfo(participantA, ev.a);
//
//	if (ImGui::BeginCombo("shape##a", aInfo.current.label.c_str()))
//	{
//		if (!IsValidCollisionParticipant(participantA))
//		{
//			ev.entity<0>() = kInvalidEntity;
//		}
//		else
//		{
//			for (const auto& infoElem : aInfo.all)
//			{
//				bool selected = (infoElem.handle == aInfo.current.handle);
//
//				if (ImGui::Selectable(infoElem.label.c_str(), &selected))
//				{
//					ev.a.entity = infoElem.entityId;
//					ev.a.shapeHandle = infoElem.handle;
//				}
//			}
//		}
//
//		ImGui::EndCombo();
//	}
//
//	ImGui::TableNextRow();
//	ImGui::TableNextColumn();
//
//	// PARTICIPANT B
//	if (ImGui::BeginCombo("participant B", curNameB.c_str()))
//	{
//		for (const auto& e : es)
//		{
//			assert(e.HasComponent<Name>());
//			const auto& name = e.GetComponent<Name>().value;
//
//			if (name == curNameA)
//			{
//				continue;
//			}
//
//			const bool selected = (name == curNameB);
//			if (ImGui::Selectable(name.c_str(), &selected))
//			{
//				ev.entity<1>() = e.GetID();
//				participantB = e;
//			}
//		}
//
//		ImGui::EndCombo();
//	}
//
//	ImGui::TableNextColumn();
//	ImGui::SetNextItemWidth(-FLT_MIN);
//
//	auto bInfo = GetCollisionDataShapeInfo(participantB, ev.b);
//
//	if (ImGui::BeginCombo("shape##b", bInfo.current.label.c_str()))
//	{
//		if (!IsValidCollisionParticipant(participantB))
//		{
//			ev.entity<1>() = kInvalidEntity;
//		}
//		else
//		{
//			for (const auto& infoElem : bInfo.all)
//			{
//				bool selected = (infoElem.handle == bInfo.current.handle);
//
//				if (ImGui::Selectable(infoElem.label.c_str(), &selected))
//				{
//					ev.b.entity = infoElem.entityId;
//					ev.b.shapeHandle = infoElem.handle;
//				}
//			}
//		}
//
//		ImGui::EndCombo();
//	}
//
//	return PropertyEditState::None;
//}

template <typename Ev>
PropertyEditState DrawCollisionEvent(Ev& ev)
{
	Entity participantA = ECS::GetEntityByID(ev.entity<0>());
	Entity participantB = ECS::GetEntityByID(ev.entity<1>());

	std::string curNameA;
	std::string curNameB;

	if (IsValidCollisionParticipant(participantA))
	{
		curNameA = participantA.GetComponent<Name>();
	}
	if (IsValidCollisionParticipant(participantB))
	{
		curNameB = participantB.GetComponent<Name>();
	}

	auto es = GetRigidBodyEntities();

	CollisionEventParticipantContext participantCtxA{
		.invisibleLabel = "##participantA",
		.allEntities = es,
		.currentName = curNameA,
		.nameToTest = curNameB,
		.participantEntity = participantA,
		.eventParticipantId = ev.entity<0>()
	};

	CollisionEventShapesContext shapesCtxA{
		.invisibleLabel = "##shapesA",
		.collisionData = ev.a,
		.participantEntity = participantA,
		.eventParticipantId = ev.entity<0>()
	};

	auto state = Property("", [&participantCtxA, &shapesCtxA] {
		return GuiEditProperties<"participant A", "shapes">(participantCtxA, shapesCtxA);
	});

	CollisionEventParticipantContext participantCtxB{
		.invisibleLabel = "##participantB",
		.allEntities = es,
		.currentName = curNameB,
		.nameToTest = curNameA,
		.participantEntity = participantB,
		.eventParticipantId = ev.entity<1>()
	};

	CollisionEventShapesContext shapesCtxB{
		.invisibleLabel = "##shapesB",
		.collisionData = ev.b,
		.participantEntity = participantB,
		.eventParticipantId = ev.entity<1>()
	};

	state |= Property("", [&participantCtxB, &shapesCtxB] {
		return GuiEditProperties<"participant B", "shapes">(participantCtxB, shapesCtxB);
	});

	return state;
}

template <typename T> requires type_in_list_v<T, events::CollisionEventGroup>
struct draw_event<T>
{
	static PropertyEditState call(T& ev)
	{
		PushPropertyDepth();

		const auto state = DrawCollisionEvent(ev);

		PopPropertyDepth();

		return state;
	}
};

template <typename T>
PropertyEditState DrawEvent(T& ev)
{
	return draw_event<T>::call(ev);
}

template <typename> struct draw_event_list;

template <template <typename...> class TList, typename...Ts>
struct draw_event_list<TList<Ts...>>
{
	static void call(EventList& editedEvents, FiredList& firedList, 
					 InspectorEventPanel::ResourceContext& ctx)
	{
		static constexpr auto impl = []<typename T>(EventList& editedEvents, FiredList& firedList,
													const GuiTextureConverter& converter, EventBus& bus)
		{
			const bool fired = firedList.Test<T>();
			firedList.Set<T>(false);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
				ImVec2(ImGui::GetStyle().FramePadding.x, 6));

			const bool open = ImGui::CollapsingHeader(GuiEventName<T>::name.data(),
				ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DrawLinesFull);

			ImGui::PopStyleVar();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

			const float buttonStartX = GetRightAlignButtonStartX(ImGui::GetFrameHeight(), 1);

			ImGui::SameLine(buttonStartX);

			const bool pressedFire = DrawFireButton<T>(converter, fired);

			ImGui::PopStyleVar();

			auto& ev = std::get<T>(editedEvents);

			if (pressedFire)
			{
				auto evCopy = ev;
				bus.PushAndDispatchEvents(std::move(evCopy));
			}

			if (!open)
			{
				return;
			}

			if (!BeginEventTable())
			{
				return;
			}

			DrawEvent(ev);

			EndEventTable();
		};

		GuiTextureConverter converter{ ctx.textureRepo };

		((impl.template operator()<Ts>(editedEvents, firedList, converter, ctx.eventBus)), ...);
	}
};

void DrawEventList(EventList& editedEvents, FiredList& firedList,
				   InspectorEventPanel::ResourceContext& ctx)
{
	draw_event_list<InspectorEventPanel::GuiEventTypeList>::call(editedEvents, firedList, ctx);
}

void InitEventFiredListCallbacks(EventBus& bus, FiredList& firedList, SignalTokenStorage& tks)
{
	init_event_fired_list_callbacks<InspectorEventPanel::GuiEventTypeList>::call(bus, firedList, tks);
}

Result<Void> InspectorEventPanel::ResetForNewScene(SceneFixture& fixture)
{
	return kVoid;
}

Result<Void> InspectorEventPanel::LoadResources(SceneFixture& fixture)
{
	TRY(ResourcePath::Sprite("ui/editor/bolt_icon.png"), boltIconPath);
	TRY(ResourcePath::Sprite("ui/editor/bolt_icon_fill.png"), boltIconFillPath);

	//auto& spriteAtlas = fixture.GetTextureRepository().GetSpriteAtlas();
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Aux TextureRepository was null");
	}
	auto& spriteAtlas = auxRepo->GetSpriteAtlas();

	TRY_ASSIGN(buttons_.fire.defaultSprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(boltIconPath) }));
	TRY_ASSIGN(buttons_.fire.activatedSprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(boltIconFillPath) }));

	return kVoid;
}

Result<Void> InspectorEventPanel::Init(SceneFixture& fixture)
{
	auto& bus = fixture.GetEventBus();

	TRY(LoadResources(fixture));

	InitEventFiredListCallbacks(bus, eventFiredList_, eventFiredTokens_);

	return kVoid;
}

void InspectorEventPanel::Update(ResourceContext& ctx)
{
	DrawEventList(editedEvents_, eventFiredList_, ctx);
}

void InspectorEventPanel::TearDown()
{
	eventFiredTokens_.signalTokens.clear();
}

} // ui

#endif