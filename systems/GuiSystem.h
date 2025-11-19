#pragma once
#include "../FeatureFlags.h"

#if IMGUI_ENABLED

#include "../gui/GuiContext.h"
#include "System.h"
#include "../deps/function2/function2.hpp"
#include "../core/Dictionary.h"

class GuiSystem : public System
{
public:
	GuiSystem() = default;

	template <typename Fn> requires std::invocable<Fn>
	bool AddWidget(std::string_view widgetName, Fn&& fn, ImGuiWindowFlags flags = 0)
	{
		auto& newWidget = widgetCommands_.emplace_back(
			PrepareWidget(widgetName, std::forward<Fn>(fn), flags)
		);
		if (!newWidget)
		{
			widgetCommands_.pop_back();
			return false;
		}

		return true;
	}

	void Update()
	{
		for (auto& cmd : widgetCommands_)
		{
			if (cmd) { std::invoke(cmd); }
		}
	}

	bool HasWidgetCommands() const { return !widgetCommands_.empty(); }

	bool* GetWidgetState(std::string_view widgetName)
	{
		auto it = widgetStates_.find(widgetName);

		return (it != widgetStates_.end()) ? &(it->second) : nullptr;
	}

	void NewFrame() { GuiContext::NewFrame(); }
	void RenderPrepare() { GuiContext::RenderPrepare(); }
	void RenderPresent(SDL_Renderer* renderer) { GuiContext::RenderPresent(renderer); }

private:
	UnorderedDictionary<bool> widgetStates_;
	std::vector<fu2::unique_function<void()>> widgetCommands_;

	template <typename Fn> requires std::convertible_to<Fn, fu2::unique_function<void()>>
	fu2::unique_function<void()> PrepareWidget(std::string_view widgetName, 
											   Fn&& fn, ImGuiWindowFlags flags)
	{
		if (widgetStates_.contains(widgetName))
		{
			return nullptr;
		}
		widgetStates_[widgetName] = true;

		return [this, flags, f = std::forward<Fn>(fn), name = std::string{ widgetName }]
		{
			auto it = widgetStates_.find(name);
			if (it != widgetStates_.end() && it->second)
			{
				ImGui::Begin(name.c_str(), &(it->second), flags);

				std::invoke(f);

				ImGui::End();
			}
		};
	}
};

#endif