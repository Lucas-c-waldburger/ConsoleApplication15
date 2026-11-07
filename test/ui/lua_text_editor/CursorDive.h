#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"

namespace ui {

template <auto fn>
concept CursorFnCheckable = requires(TreeCursor & c)
{
	{ (c.*fn)() } -> std::same_as<bool>;
};
template <auto fn, typename T>
concept CursorFnCheckableAgainst = requires(TreeCursor & c)
{
	{ (c.*fn)() } -> std::equality_comparable_with<T>;
};
template <auto fn>
concept NodeFnCheckable = requires(TreeCursor & c)
{
	{ ((c.CurrentNode()).*fn)() } -> std::same_as<bool>;
};
template <auto fn, typename T>
concept NodeFnCheckableAgainst = requires(TreeCursor & c)
{
	{ ((c.CurrentNode()).*fn)() } -> std::equality_comparable_with<T>;
};

template <auto fn>
concept CursorDiveCheckable = CursorFnCheckable<fn> || NodeFnCheckable<fn>;
template <auto fn, typename T>
concept CursorDiveCheckableAgainst = CursorFnCheckableAgainst<fn, T> ||
									 NodeFnCheckableAgainst<fn, T>;

struct CursorDive
{
	explicit CursorDive(TreeCursor& c) : cursor(c), good(c), depth(0) {}
	~CursorDive()
	{
		while (depth-- > 0 && cursor.GoToParent()) {}
	}
	CursorDive(const CursorDive& other) : 
		cursor(other.cursor), good(other.good), depth(0) {}

	TreeCursor& cursor;
	bool good;
	int depth;

	operator bool() const { return good; }
	bool operator!() const { return !good; }

	SourceLocation Location() const { return cursor.CurrentNode().GetSourceLocation(); }
	std::string_view Type() const { return cursor.CurrentNode().GetType(); }
	std::string_view Text() const { return cursor.CurrentNode().GetText(); }
	TreeNode Node() const { return cursor.CurrentNode(); }

	CursorDive& ToChild()
	{
		if (good && (good = cursor.GoToFirstChild())) { ++depth; }
		return *this;
	}

	template <auto fn>
		requires std::is_invocable_r_v<bool, decltype(fn), const TreeNode&>
	CursorDive& ToChild()
	{
		if (!good || !cursor.GoToFirstChild()) { return *this; }

		++depth;

		do
		{
			if (good = fn(cursor.CurrentNode())) { return *this; }

		} while (cursor.GoToNextSibling());

		good = false;

		return *this;
	}


	template <auto fn, typename T> requires CursorDiveCheckableAgainst<fn, T>
	CursorDive& ToChild(const T& t)
	{
		if (!good || !cursor.GoToFirstChild()) { return *this; }

		++depth;

		do
		{
			if constexpr (CursorFnCheckableAgainst<fn, T>)
			{
				if (good = ((cursor.*fn)() == t)) { return *this; }
			}
			else if constexpr (NodeFnCheckableAgainst<fn, T>)
			{
				if (good = (((cursor.CurrentNode()).*fn)() == t)) { return *this; }
			}
		} while (cursor.GoToNextSibling());

		good = false;

		return *this;
	}

	template <auto fn1, auto fn2, typename T, typename U> 
		requires CursorDiveCheckableAgainst<fn1, T> && CursorDiveCheckableAgainst<fn2, U>
	CursorDive& ToChild(const T& t, const U& u)
	{
		if (!good || !cursor.GoToFirstChild()) { return *this; }

		++depth;

		do
		{
			if constexpr (CursorFnCheckableAgainst<fn1, T>)
			{
				if (good = ((cursor.*fn1)() == t)) 
				{ 
					if constexpr (CursorFnCheckableAgainst<fn2, U>)
					{
						if (good = ((cursor.*fn2)() == u)) { return *this; }
					}
					else if constexpr (NodeFnCheckableAgainst<fn2, U>)
					{
						if (good = (((cursor.CurrentNode()).*fn2)() == u)) { return *this; }
					}
				}
			}
			else if constexpr (NodeFnCheckableAgainst<fn1, T>)
			{
				if (good = (((cursor.CurrentNode()).*fn1)() == t))
				{
					if constexpr (CursorFnCheckableAgainst<fn2, U>)
					{
						if (good = ((cursor.*fn2)() == u)) { return *this; }
					}
					else if constexpr (NodeFnCheckableAgainst<fn2, U>)
					{
						if (good = (((cursor.CurrentNode()).*fn2)() == u)) { return *this; }
					}
				}
			}
		} while (cursor.GoToNextSibling());

		good = false;

		return *this;
	}

	CursorDive& ToNamedChild()
	{
		if (good && (good = cursor.GoToFirstNamedChild())) { ++depth; }
		return *this;
	}

	CursorDive& ToParent()
	{
		if (good && (good = cursor.GoToParent())) { --depth; }
		return *this;
	}

	CursorDive& ToParent(size_t count)
	{
		if (good)
		{
			for (size_t i = 0; i < std::min(count, static_cast<size_t>(depth)); ++i)
			{
				if (!(good = cursor.GoToParent()))
				{
					break;
				}
				--depth;
			}
		}
		return *this;
	}

	CursorDive& ToSibling()
	{
		if (good && (good = cursor.GoToNextSibling())) {}
		return *this;
	}

	CursorDive& ToNamedSibling()
	{
		if (good && (good = cursor.GoToNextNamedSibling())) {}
		return *this;
	}

	template <auto fn, typename T> requires CursorDiveCheckableAgainst<fn, T>
	CursorDive& ToSibling(const T& t)
	{
		if (!good) { return *this; }

		while (cursor.GoToNextSibling())
		{
			if constexpr (CursorFnCheckableAgainst<fn, T>)
			{
				if (((cursor.*fn)() == t))
				{
					return *this;
				}
			}
			else if constexpr (NodeFnCheckableAgainst<fn, T>)
			{
				if ((((cursor.CurrentNode()).*fn)() == t))
				{
					return *this;
				}
			}
		}
		good = false;
		return *this;
	}

	template <auto fn1, auto fn2, typename T, typename U> 
		requires CursorDiveCheckableAgainst<fn1, T> && CursorDiveCheckableAgainst<fn2, U>
	CursorDive& ToSibling(const T& t, const U& u)
	{
		if (!good) { return *this; }

		while (cursor.GoToNextSibling())
		{
			if constexpr (CursorFnCheckableAgainst<fn1, T>)
			{
				if (((cursor.*fn1)() == t))
				{
					if constexpr (CursorFnCheckableAgainst<fn2, U>)
					{
						if (((cursor.*fn2)() == u))
						{
							return *this;
						}
					}
					else if constexpr (NodeFnCheckableAgainst<fn2, U>)
					{
						if ((((cursor.CurrentNode()).*fn2)() == u))
						{
							return *this;
						}
					}

					return *this;
				}
			}
			else if constexpr (NodeFnCheckableAgainst<fn1, T>)
			{
				if ((((cursor.CurrentNode()).*fn1)() == t))
				{
					if constexpr (CursorFnCheckableAgainst<fn2, U>)
					{
						if (((cursor.*fn2)() == u))
						{
							return *this;
						}
					}
					else if constexpr (NodeFnCheckableAgainst<fn2, U>)
					{
						if ((((cursor.CurrentNode()).*fn2)() == u))
						{
							return *this;
						}
					}
					return *this;
				}
			}
		}
		good = false;
		return *this;
	}

	template <typename Fn> requires std::is_invocable_r_v<bool, Fn, const TreeNode&>
	CursorDive& ToSiblingIf(Fn&& pred)
	{
		if (!good) { return *this; }

		while (cursor.GoToNextSibling())
		{
			if (pred(cursor.CurrentNode()))
			{
				return *this;
			}
		}

		good = false;
		return *this;
	}

	template <auto fn> requires CursorDiveCheckable<fn>
	CursorDive& Check()
	{
		if (!good) { return *this; }

		if constexpr (CursorFnCheckable<fn>)
		{
			good = (cursor.*fn)();
		}
		else if constexpr (NodeFnCheckable<fn>)
		{
			good = (((cursor.CurrentNode()).*fn)());
		}

		return *this;
	}

	template <auto fn, typename T> requires CursorDiveCheckableAgainst<fn, T>
	CursorDive& Check(const T& t)
	{
		if (!good) { return *this; }

		if constexpr (CursorFnCheckableAgainst<fn, T>)
		{
			good = ((cursor.*fn)() == t);
		}
		else if constexpr (NodeFnCheckableAgainst<fn, T>)
		{
			good = ((((cursor.CurrentNode()).*fn)() == t));
		}

		return *this;
	}

	template <auto fn> 
		requires std::is_invocable_r_v<bool, decltype(fn), const TreeNode&>
	CursorDive& Check()
	{
		good = (good && fn(cursor.CurrentNode()));

		return *this;
	}
};

} // ui

#endif
