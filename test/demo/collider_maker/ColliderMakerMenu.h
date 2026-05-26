#pragma once
#include "ColliderMaker2.h"
#include "ColliderMakerCommon.h"
#include "../../../core/CommonFunctions.h"

namespace test {

class NavigationStack;

class ColliderMakerMenu
{
public:
	using UniquePtr = std::unique_ptr<ColliderMakerMenu>;

	virtual ~ColliderMakerMenu() = default;

	const MenuType GetMenuType() const noexcept { return menuType_; }
	const std::string& GetIdentifier() const noexcept { return identifier_; }

	virtual void Draw(NavigationStack& navStack) = 0;

protected:
	explicit ColliderMakerMenu(const MenuType& mt, std::string_view ident) :
		menuType_(mt), identifier_(ident) {}

private:
	MenuType menuType_ = MenuType::Unknown;
	std::string identifier_;
};


} // test