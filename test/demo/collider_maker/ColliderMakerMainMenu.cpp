#include "ColliderMakerMainMenu.h"


namespace test {

void ColliderMakerMainMenu::Draw(NavigationStack& navStack)
{
	auto it = layerIdentifiers_.begin();
	for (size_t i = 0; i < layerIdentifiers_.size(); ++i)
	{
		ImGui::PushID(i);

		const auto& layerIdent = *(++it);

		if (ImGui::Button(layerIdent.data()))
		{
			selectedLayer_ = layerIdent;
		}

		ImGui::PopID();
	}
}







} // 