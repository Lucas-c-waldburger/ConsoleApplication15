#pragma once
#include "ColliderMakerMenu.h"

namespace test {

class NavigationStack
{
public:
	NavigationStack() = default;

	template <typename T, typename...Args>
		requires (std::derived_from<T, ColliderMakerMenu>&&
	std::constructible_from<T, Args...>)
		void Push(const Args&&...args)
	{
		AssertValid();

		stack_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		++head_;
	}

	void Pop()
	{
		AssertValid();
		
		--head_;
	}

	const ColliderMakerMenu::UniquePtr& GetCurrent() const
	{
		AssertValid();

		return stack_[head_];
	}

	size_t Size() const noexcept 
	{ 
		AssertValid();

		return stack_.size(); 
	}

	void Draw()
	{
		AssertValid();

		if (stack_.size() > 1)
		{
			if (ImGui::Button("< Back"))
			{
				Pop();
			}
		}

		stack_.back()->Draw(*this);
	}

private:
	void AssertValid() const
	{
		assert(!stack_.empty());
		assert(head_ == stack_.size() - 1);
		assert(stack_[head_]);
	}

	size_t head_ = 0;
	std::vector<ColliderMakerMenu::UniquePtr> stack_;
};

} // test