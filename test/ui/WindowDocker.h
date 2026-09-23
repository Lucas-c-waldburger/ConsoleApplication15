#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../core/commonObjects.h"
#include "InspectorCommon.h"
#include <imgui_internal.h>
#include <bit>

namespace ui {

inline constexpr ImGuiDir GetOppositeDirection(ImGuiDir dir)
{
	switch (dir)
	{
	case ImGuiDir_Up:    return ImGuiDir_Down;
	case ImGuiDir_Down:  return ImGuiDir_Up;
	case ImGuiDir_Left:  return ImGuiDir_Right;
	case ImGuiDir_Right: return ImGuiDir_Left;
	default:             return ImGuiDir_None;
	}
}

inline constexpr ImGuiDir GetPerpendicularDirection(ImGuiDir dir)
{
	switch (dir)
	{
	case ImGuiDir_Up:
	case ImGuiDir_Down:
		return ImGuiDir_Left;
	case ImGuiDir_Left:
	case ImGuiDir_Right:
	default:
		return ImGuiDir_Down;
	}
}

struct DockNode;

struct DockPlacement
{
	DockNode* target = nullptr;
	ImGuiDir direction = ImGuiDir_None;
};

struct DockNode
{
	std::vector<DockNode> regions;
	EditorWindowType window = static_cast<EditorWindowType>(0);
	ImGuiID id = 0;
	ImGuiDir direction = ImGuiDir_None;
	size_t splitCount = 0;

	bool IsLeaf() const
	{
		return regions.empty();
	}

	DockNode* Split(ImGuiDir dir, float ratio)
	{
		assert(IsLeaf());
		assert(id != 0);

		ImGuiID newNodeId;
		ImGuiID currentNodeId;

		ImGui::DockBuilderSplitNode(
			id,
			dir,
			ratio,
			&newNodeId,
			&currentNodeId);

		DockNode oldNode{
			.regions = {},
			.window = window,
			.id = currentNodeId,
			.direction = ImGuiDir_None,
			.splitCount = 0
		};

		direction = dir;
		window = static_cast<EditorWindowType>(0);

		regions.clear();
		regions.push_back(std::move(oldNode));
		regions.push_back(DockNode{
			.regions = {},
			.window = static_cast<EditorWindowType>(0),
			.id = newNodeId,
			.direction = ImGuiDir_None,
			.splitCount = 0
		});

		return &regions.back();
	}
};

struct DockNode2
{
	struct ChildPair
	{
		DockNode2* oldNode = nullptr;
		DockNode2* newNode = nullptr;
	};

	std::vector<DockNode2> children;
	ImGuiID id = 0;
	EditorWindowType window = static_cast<EditorWindowType>(0);
	ImGuiDir direction = ImGuiDir_None;

	DockNode2* Split(ImGuiDir dir, float ratio)
	{
		assert(children.empty());
		assert(id != 0);

		ImGuiID newNodeId;
		ImGuiID currentNodeId;

		ImGui::DockBuilderSplitNode(
			id,
			dir,
			ratio,
			&newNodeId,
			&currentNodeId);

		DockNode2 oldNode{
			.children = {},
			.id = currentNodeId,
			.window = window,
			.direction = GetOppositeDirection(dir)
		};

		id = 0;
		window = static_cast<EditorWindowType>(0);

		children.clear();
		children.push_back(std::move(oldNode));
		children.push_back(DockNode2{
			.children = {},
			.id = newNodeId,
			.window = static_cast<EditorWindowType>(0),
			.direction = dir
		});

		return &children.back();
	}
};

struct DockRegion
{
	DockNode2* headNode = nullptr;
	ImGuiDir regionDirection = ImGuiDir_None;
	ImGuiDir nextNodeDirection = ImGuiDir_None;
}; 

class WindowDocker2
{
public:
	void Init()
	{
		Clear();

		dockspaceId_ = ImGui::GetID("MainDockSpace");

		ImGui::DockBuilderRemoveNode(dockspaceId_);

		ImGui::DockBuilderAddNode(
			dockspaceId_,
			ImGuiDockNodeFlags_DockSpace);

		ImGui::DockBuilderSetNodeSize(
			dockspaceId_,
			ImGui::GetMainViewport()->WorkSize);

		rootNode_ = DockNode2{
			.children = {},
			.id = dockspaceId_,
			.window = EditorWindowType::GameWindow,
			.direction = ImGuiDir_None
		};

		dockedWindowTypes_ |= EditorWindowType::GameWindow;

		ImGui::DockBuilderDockWindow(
			GetEditorWindowName(EditorWindowType::GameWindow).data(),
			dockspaceId_);

		ImGui::DockBuilderFinish(dockspaceId_);
	}

	ImGuiID GetDockspaceID() const noexcept { return dockspaceId_; }

	void DockWindow(EditorWindowType windowType)
	{
		if (dockedWindowTypes_ & windowType)
		{
			return;
		}

		const auto windowCount = std::popcount(dockedWindowTypes_);

		auto* node = GetDockNode(windowCount);
		DockNode2* nodeToSplit = nullptr;
		bool reassignDockRegionNode = windowCount <= 3;

		if (node->children.empty())
		{
			nodeToSplit = node;
		}
		else if (node->children[0].children.empty())
		{
			nodeToSplit = &node->children[0];
		}
		else
		{
			assert(node->children[1].children.empty());

			nodeToSplit = &node->children[1];
			reassignDockRegionNode = true;
		}

		const auto dir = GetNextNodeDirection(nodeToSplit, windowCount);
		assert(dir != ImGuiDir_None);

		auto* newNode = nodeToSplit->Split(dir, GetSplitRatio(windowCount));

		newNode->window = windowType;

		if (reassignDockRegionNode)
		{
			SetDockRegionNode(nodeToSplit, windowCount);
		}
	
		dockedWindowTypes_ |= windowType;

		ImGui::DockBuilderDockWindow(
			GetEditorWindowName(windowType).data(),
			newNode->id);

		ImGui::DockBuilderFinish(dockspaceId_);
	}

	void Clear()
	{
		leftRegion_ = {};
		rightRegion_ = {};
		bottomRegion_ = {};
		dockspaceId_ = 0;
		rootNode_ = {};
		dockedWindowTypes_ = 0;
	}

private:
	static constexpr float GetSplitRatio(size_t windowCount)
	{
		assert(windowCount > 0);

		switch (windowCount)
		{
		case 1: return 0.2f;
		case 2: return 0.25f;
		case 3: return 0.3f;
		default: return 0.5f;
		}

		std::unreachable();
	}

	void SetDockRegionNode(DockNode2* node, size_t windowCount)
	{
		assert(windowCount > 0);

		switch ((windowCount - 1) % 3)
		{
		case 0: leftRegion_ = node; return;
		case 1: rightRegion_ = node; return;
		case 2: bottomRegion_ = node; return;
		}

		std::unreachable();
	}

	static constexpr ImGuiDir GetNextNodeDirection(DockNode2* node, size_t windowCount)
	{
		assert(node);
		assert(windowCount > 0);

		switch (windowCount)
		{
		case 1: return ImGuiDir_Left;
		case 2: return ImGuiDir_Right;
		case 3: return ImGuiDir_Down;
		default: return GetPerpendicularDirection(node->direction);
		}

		std::unreachable();
	}

	DockNode2* GetDockNode(size_t windowCount)
	{
		assert(windowCount > 0);

		switch (windowCount)
		{
		case 1: return &rootNode_;
		case 2: case 3: return FindGameWindowNode();
		default :
		{
			switch ((windowCount - 1) % 3)
			{
			case 0: return leftRegion_;
			case 1: return rightRegion_;
			case 2: return bottomRegion_;
			}

			std::unreachable();
		}
		}

		std::unreachable();
	}

	DockNode2* FindGameWindowNode()
	{
		return FindGameWindowNodeImpl(&rootNode_);
	}

	DockNode2* FindGameWindowNodeImpl(DockNode2* node)
	{
		if (!node)
		{
			return nullptr;
		}

		if (node->window & EditorWindowType::GameWindow)
		{
			return node;
		}
		else
		{
			for (auto& child : node->children)
			{
				if (auto* gameNode = FindGameWindowNodeImpl(&child))
				{
					return gameNode;
				}
			}
		}

		return nullptr;
	}

	DockNode2* leftRegion_;
	DockNode2* rightRegion_;
	DockNode2* bottomRegion_;
	DockNode2 rootNode_;
	ImGuiID dockspaceId_ = 0;
	uint8_t dockedWindowTypes_ = 0;
};

class WindowDocker
{
public:
	static constexpr std::array kDirTraversal = {
		ImGuiDir_Left,
		ImGuiDir_Right,
		ImGuiDir_Down
	};

	enum DirEncounter : uint8_t
	{
		Left = 1 << 0,
		Right = 1 << 1,
		Down = 1 << 2
	};

	void Init()
	{
		Clear();

		dockspaceId_ = ImGui::GetID("MainDockSpace");

		ImGui::DockBuilderRemoveNode(dockspaceId_);

		ImGui::DockBuilderAddNode(
			dockspaceId_,
			ImGuiDockNodeFlags_DockSpace);

		ImGui::DockBuilderSetNodeSize(
			dockspaceId_,
			ImGui::GetMainViewport()->WorkSize);

		rootNode_ = DockNode{				
			.regions = {},
			.window = EditorWindowType::GameWindow,
			.id = dockspaceId_,
			.direction = ImGuiDir_None,
			.splitCount = 0
		};

		dockedWindowTypes_ |= EditorWindowType::GameWindow;

		ImGui::DockBuilderDockWindow(
			GetEditorWindowName(EditorWindowType::GameWindow).data(), 
			dockspaceId_);

		ImGui::DockBuilderFinish(dockspaceId_);
	}

	void DockWindow(EditorWindowType windowType)
	{
		if (dockedWindowTypes_ & windowType)
		{
			return;
		}

		const auto windowCount = std::popcount(dockedWindowTypes_);

		auto placement = ChoosePlacementTarget(windowCount);

		assert(placement.target);
		++placement.target->splitCount;

		DockNode* newNode = placement.target->Split(
			placement.direction,
			(windowCount > 4 ? 0.5f : 0.2f)
		);

		newNode->window = windowType;
		dockedWindowTypes_ |= windowType;

		//switch (windowCount)
		//{
		//case 1:
		//	splits_.leftRegion = windowType; break;
		//case 2:
		//	splits_.rightRegion = windowType; break;
		//case 3:
		//	splits_.botRegion = windowType; break;
		//default:
		//	break;
		//}

		ImGui::DockBuilderDockWindow(
			GetEditorWindowName(windowType).data(),
			newNode->id);

		ImGui::DockBuilderFinish(dockspaceId_);
	}

	void Clear()
	{
		dockspaceId_ = 0;
		rootNode_ = {};
		dockedWindowTypes_ = 0;
	}

	ImGuiID GetDockspaceID() const noexcept { return dockspaceId_; }

private:
	DockPlacement ChoosePlacementTarget(size_t windowCount)
	{
		//const auto windowCount = std::popcount(dockedWindowTypes_);

		switch (windowCount)
		{
		case 1:
			return {
				.target = &rootNode_,
				.direction = ImGuiDir_Left
			};
		case 2:
			return {
				.target = FindGameWindowNode(),
				.direction = ImGuiDir_Right
			};
		case 3:
			return {
				.target = FindGameWindowNode(),
				.direction = ImGuiDir_Down
			};
		default:
		{
			//auto* splitNode = GetSplitNode(windowCount);
			//auto placement = FindAvailablePlacement(splitNode);
			auto placement = FindAvailablePlacement(&rootNode_);
			assert(placement.target);
			assert(placement.direction != ImGuiDir_None);

			placement.direction = GetPerpendicularDirection(placement.direction);
			//placement.direction = GetSplitDirection(windowCount);

			return placement;
		}
		}
	}

	size_t CountWindows(const DockNode& node) const
	{
		if (node.IsLeaf())
		{
			return node.window != static_cast<EditorWindowType>(0) ? 1 : 0;
		}

		size_t count = 0;

		for (const auto& child : node.regions)
		{
			count += CountWindows(child);
		}

		return count;
	}

	//DockNode* FindSplit(ImGuiDir splitDir)
	//{

	//}

	//DockNode* FindSplitImpl(DockNode* node, ImGuiDir splitDir)
	//{
	//	if (node)
	//	{
	//		for (auto& region : node->regions)
	//		{
	//			if (auto* splitNode = FindSplitImpl(&region, splitDir))
	//			{
	//				return splitNode;
	//			}
	//		}

	//		if (node->direction == splitDir)
	//		{
	//			return node;
	//		}
	//	}

	//	return node;
	//}

	DockNode* GetSplitNode(size_t windowCount)
	{
		assert(windowCount >= 1);

		switch ((windowCount - 1) % 3)
		{
		case 0:
			return FindNode(splits_.leftRegion);
		case 1:
			return FindNode(splits_.rightRegion);
		case 2:
		default:
			return FindNode(splits_.botRegion);
		}
	}

	DockNode* FindGameWindowNode()
	{
		return FindGameWindowNodeImpl(&rootNode_);
	}

	DockNode* FindNode(EditorWindowType windowType)
	{
		return FindNodeImpl(&rootNode_, windowType);
	}

	DockNode* FindNodeImpl(DockNode* node, EditorWindowType windowType)
	{
		if (!node)
		{
			return nullptr;
		}

		if (node->window == windowType)
		{
			return node;
		}
		else
		{
			for (auto& child : node->regions)
			{
				if (auto* curNode = FindNodeImpl(&child, windowType))
				{
					return curNode;
				}
			}
		}

		return nullptr;
	}

	DockPlacement FindAvailablePlacement(DockNode* node)
	{
		assert(node);

		DockPlacement dockPlacement{};

		FindAvailablePlacementImpl(node, dockPlacement);

		assert(dockPlacement.target);
		assert(dockPlacement.direction != ImGuiDir_None);

		return dockPlacement;
	}

	//std::vector<DockNode*> GatherAvailableNodes() 
	//{

	//}

	void FindAvailablePlacementImpl(DockNode* node, DockPlacement& placement)
	{
		if (node->IsLeaf())
		{
			if (node->window != EditorWindowType::GameWindow)
			{
				if (!placement.target || placement.target->splitCount >=
					node->splitCount)
				{
					placement.target = node;
				}
			}

			return;
		}

		if (placement.target)
		{
			return;
		}

		placement.direction = node->direction;

		//assert(node->regions.size() == 2);
		//const size_t first = node->regions[0].splitCount < node->regions[1].splitCount ?
		//					 0 : 1;
		//const size_t second = first == 0 ? 1 : 0;

		//auto placement1 = placement;
		//auto placement2 = placement;

		//FindAvailablePlacementImpl(&node->regions[first], placement1);
		//FindAvailablePlacementImpl(&node->regions[second], placement2);
		//if (placement.target)
		//{
		//	++node->splitCount;
		//	return;
		//}

		for (auto& region : node->regions)
		{
			FindAvailablePlacementImpl(&region, placement);
			if (placement.target)
			{
				++node->splitCount;
				return;
			}
		}
	}

	DockNode* FindGameWindowNodeImpl(DockNode* node)
	{
		if (!node)
		{
			return nullptr;
		}

		if (node->window & EditorWindowType::GameWindow)
		{
			return node;
		}
		else
		{
			for (auto& child : node->regions)
			{
				if (auto* curNode = FindGameWindowNodeImpl(&child))
				{
					return curNode;
				}
			}
		}

		return nullptr;
	}

	//struct {
	//	DockNode* leftRegion = nullptr;
	//	DockNode* rightRegion = nullptr;
	//	DockNode* botRegion = nullptr;
	//} splits_;
	//struct {
	//	ImGuiID leftRegion = 0;
	//	ImGuiID rightRegion = 0;
	//	ImGuiID botRegion = 0;
	//} splits_;
	struct {
		EditorWindowType leftRegion = static_cast<EditorWindowType>(0);
		EditorWindowType rightRegion = static_cast<EditorWindowType>(0);
		EditorWindowType botRegion = static_cast<EditorWindowType>(0);
	} splits_;
	ImGuiID dockspaceId_ = 0;
	DockNode rootNode_;
	uint8_t dockedWindowTypes_ = 0;
	size_t dirTraversalIndex_ = 0;
};

} // ui

#endif