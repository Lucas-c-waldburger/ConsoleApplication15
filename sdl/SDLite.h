#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <vector>
#include <cassert>

namespace SDLite
{
	struct Status
	{
		enum class Response
		{
			Warn = 1,
			Throw,
			Exit
		};

		Status() = default;
		Status(std::string msg, Response resp = Response::Warn) : message_(std::move(msg)) {}

		bool Good() const { return message_.empty(); }
		const std::string& GetMessage() const { return message_; }
		Response GetResponse() const { return response_; }
		bool Throws() const { return response_ == Response::Throw; }

	private:
		std::string message_;
		Response response_ = Response::Warn;
	};

	static void HandleStatus(const Status& status)
	{
		if (status.Good()) { return; }

		switch (status.GetResponse())
		{
		case Status::Response::Warn:
			SDL_Log(status.GetMessage().c_str()); break;
		case Status::Response::Throw:
			throw std::runtime_error(status.GetMessage());
		case Status::Response::Exit:
			std::cerr << status.GetMessage();
			std::exit(EXIT_FAILURE);
		}
	}

	template <typename T, typename Arg>
	using AppPtrCtor = T* (*)(Arg);

	template <typename T>
	using AppPtrDel = void (*)(T*);

	template <typename T, typename CtorArg, AppPtrCtor<T, CtorArg> Ctor, AppPtrDel<T> Del>
	struct AppPtr
	{
		using WrappedType = T*;
		using PtrType = std::unique_ptr<T, decltype(Del)>;
		using CtorArgType = CtorArg;

		static PtrType Create(CtorArg&& arg) { return PtrType{ Ctor(std::forward<CtorArg>(arg)), Del }; }
		static PtrType MakeEmpty() { return PtrType{ nullptr, Del }; }
	};

	template <typename AppPtrT>
	class AppObject
	{
	public:
		AppObject() : appPtr_(typename AppPtrT::MakeEmpty()) {}
		~AppObject() = default;
		AppObject(const AppObject&) = delete;
		AppObject& operator=(const AppObject&) = delete;
		AppObject(AppObject&& rhs) noexcept : appPtr_(std::move(rhs.appPtr_)) {}
		AppObject& operator=(AppObject&& rhs) noexcept {
			if (&rhs != this) { appPtr_ = std::move(appPtr_); }
			return *this;
		}

		operator typename AppPtrT::WrappedType() { return appPtr_.get(); }

		const Status& GetStatus() const { return status_; }

	private:
		typename AppPtrT::PtrType appPtr_;
		Status status_;

	protected:
		const Status& Init(typename AppPtrT::CtorArgType arg)
		{
			if (appPtr_)
			{
				status_ = Status{ "App object already initialized", Status::Response::Warn };

				return status_;
			}

			appPtr_ = typename AppPtrT::Create(std::move(arg));

			status_ = (appPtr_ == nullptr) ? Status{ SDL_GetError(), Status::Response::Exit } : Status{};

			return status_;
		}

	};

	static constexpr int kWindowWidth = 1200;
	static constexpr int kWindowHeight = 900;

	struct WindowArgs
	{
		int w = 1200;
		int h = 900;
		int x = SDL_WINDOWPOS_UNDEFINED;
		int y = SDL_WINDOWPOS_UNDEFINED;
		std::string title;
		Uint32 flags = SDL_WINDOW_SHOWN;
	};

	using WindowPtr = AppPtr<SDL_Window, WindowArgs,
		[](WindowArgs wa) { return SDL_CreateWindow(wa.title.c_str(), wa.x, wa.y, wa.w, wa.h, wa.flags); },
		[](SDL_Window* win) { SDL_DestroyWindow(win); }
	>;

	class AppWindow : public AppObject<WindowPtr>
	{
	public:
		friend Status Start(WindowArgs);

	private:
		AppWindow() : AppObject() {}
		WindowPtr windowPtr_;
	};

	using RendererPtr = AppPtr<SDL_Renderer, SDL_Window*,
		[](SDL_Window* win) { return SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); },
		[](SDL_Renderer* ren) { SDL_DestroyRenderer(ren); }
	>;

	class AppRenderer : public AppObject<RendererPtr>
	{
	public:
		friend Status Start(WindowArgs);

		void Clear(SDL_Color clearClr = { 0xFF, 0xFF, 0xFF, 0xFF })
		{
			SDL_SetRenderDrawColor(*this, clearClr.r, clearClr.g, clearClr.b, clearClr.a);
			SDL_RenderClear(*this);
		}

		void Show()
		{
			SDL_RenderPresent(*this);
		}

		SDL_Color GetColor()
		{
			SDL_Color clr;
			SDL_GetRenderDrawColor(*this, &clr.r, &clr.g, &clr.b, &clr.a);

			return clr;
		}

		void SetColor(SDL_Color clr)
		{
			SDL_SetRenderDrawColor(*this, clr.r, clr.g, clr.b, clr.a);
		}

	private:
		AppRenderer() : AppObject() {}
	};

	struct TextureArgs
	{
		SDL_Renderer* renderer = nullptr;
		SDL_Surface* surface = nullptr;
	};

	using TexturePtr = AppPtr<SDL_Texture, TextureArgs,
		[](TextureArgs args) { return SDL_CreateTextureFromSurface(args.renderer, args.surface); },
		[](SDL_Texture* tx) { SDL_DestroyTexture(tx); }
	>;

	class Texture : public AppObject<TexturePtr>
	{
	public:
		Texture() : AppObject() {}

		static Texture Create(TextureArgs args)
		{
			Texture tx{};
			HandleStatus(tx.Init(std::move(args)));

			return tx;
		}
	};

	template <typename AppObj>
	class AppCallback
	{
	public:
		struct RunTracker
		{
			int numTimes;
			std::function<bool()> untilFn;
		};

		using Fn = std::function<void()>;
		using Pred = std::function<bool()>;

		AppCallback& Run(Fn&& fn)
		{
			func_ = std::move(fn);
			return *this;
		}

		AppCallback& For(int numTimes)
		{
			tracker_.numTimes = numTimes;
			return *this;
		}

		AppCallback& Until(Pred&& pred)
		{
			tracker_.untilFn = std::move(pred);
			return *this;
		}

		bool Done() const { return tracker_ == 0; }

		void operator--() { --tracker_.numTimes; }

	private:
		Fn func_;
		RunTracker tracker_;
	};

	class AppEvents
	{
	public:
		bool Process()
		{
			while (SDL_PollEvent(&ev_))
			{
				if (ev_.type == SDL_QUIT)
				{
					return false;
				}
			}

			return true;
		}

	private:
		SDL_Event ev_;
	};

	class AppScripts
	{
	public:
		//void AddScript(std::string scr)
		//{
		//	lua_.script(std::move(scr));
		//}

		//sol::function operator[](std::string_view nm)
		//{
		//	return lua_[nm];
		//}

		//template <typename T>    
		//void RegisterType()

		//template <typename T>

		//template <typename T, typename...Args>
		//T Run(std::string_view fnName, Args&&...args)
		//{
		//	return static_cast<T>(lua_[fnName](std::forward<Args>(args)...));
		//}

	private:
		//sol::state lua_;
	};

	class CanvasObject
	{
	public:
		friend class AppCanvas;
		using SharedPtr = std::shared_ptr<CanvasObject>;
		virtual ~CanvasObject() = default;

		virtual void Move(int newX, int newY) = 0;
		virtual void SetPos(int newX, int newY) = 0;

	private:
		virtual void Draw(AppRenderer&) = 0;
	};

	struct Color
	{
		enum Option { Outline, Fill };
		SDL_Color values;
		Option option = Fill;
	};

	class Geometry : public virtual CanvasObject
	{
	public:
		virtual ~Geometry() = default;

		virtual void Move(int newX, int newY) = 0;
		virtual void SetPos(int newX, int newY) = 0;

		Color color;

	private:
		virtual void Draw(AppRenderer&) = 0;
	};

	class Rectangle : public Geometry, public SDL_Rect
	{
	public:
		operator SDL_Rect*() { return this; }
		virtual ~Rectangle() = default;

		void Move(int newX, int newY) override { x += newX; y += newY; }
		void SetPos(int newX, int newY) override { x = newX; y = newY; }

	private:
		void Draw(AppRenderer& renderer) override
		{
			renderer.SetColor(color.values);

			(color.option == Color::Fill) ? SDL_RenderFillRect(renderer, this) :
											SDL_RenderDrawRect(renderer, this);
		}
	};

	class Point : public Geometry, public SDL_Point
	{
	public:
		operator SDL_Point* () { return this; }
		virtual ~Point() = default;

		void Move(int newX, int newY) override { x += newX; y += newY; }
		void SetPos(int newX, int newY) override { x = newX; y = newY; }

	private:
		void Draw(AppRenderer& renderer) override
		{
			renderer.SetColor(color.values);
			SDL_RenderDrawPoint(renderer, x, y);
		}
	};

	struct Line : public Geometry
	{
	public:
		virtual ~Line() = default;

		void Move(int newX, int newY) override 
		{ 
			for (auto& p : points) { p.Move(newX, newY); }
		}

		void SetPos(int newX, int newY) override 
		{ 
			if (points.empty()) { return; }

			auto& anchor = points.front();
			int xDiff = newX - anchor.x;
			int yDiff = newY - anchor.y;

			Move(xDiff, yDiff);
		}

	private:
		std::vector<Point> points;
		void Draw(AppRenderer& renderer) override
		{
			renderer.SetColor(color.values);
			SDL_RenderDrawLines(renderer, points.data(), points.size());
		}
	};

	template <typename T>
	concept CanvasObjectType = std::derived_from<T, CanvasObject>;

	class AppCanvas
	{
	public:
		template <CanvasObjectType T>
		CanvasObject::SharedPtr AddObject(T&& obj)
		{
			return objects_.emplace_back(std::make_shared<T>(std::move(obj)));
		}

		bool RemoveObject(const CanvasObject::SharedPtr& obj)
		{
			return std::erase(objects_, obj);
		}

		void Draw(AppRenderer& renderer)
		{
			SDL_Color currentClr = renderer.GetColor();

			for (auto& obj : objects_)
			{
				if (!obj)
				{
					continue;
				}

				obj->Draw(renderer);
			}

			renderer.SetColor(currentClr);
		}

		std::vector<CanvasObject::SharedPtr> objects_;
	};

	class App
	{
	public:
		friend Status Start(WindowArgs);
		friend void Exit();

		friend AppWindow& Window();
		friend AppRenderer& Renderer();
		friend AppCanvas& Canvas();
		friend AppEvents& Events();
		friend AppScripts& Scripts();

		~App() { if (app_) { delete app_; } }
		App(const App&) = delete;
		App(App&&) = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&) = delete;

	private:
		App() = default;
		App(AppWindow&& win, AppRenderer&& rend) : window_(std::move(win)), renderer_(std::move(rend)) {}

		static inline App* app_ = nullptr;

		AppWindow window_;
		AppRenderer renderer_;
		AppCanvas canvas_;
		AppEvents events_;
	};

#define TRY_RET_APP_MEMBER(appMember) do { \
	if (!App::app_) { throw std::runtime_error("App not initialized"); } \
	return App::app_->appMember; \
} while(0)

	static AppWindow& Window()
	{
		TRY_RET_APP_MEMBER(window_);
	}

	static AppRenderer& Renderer()
	{
		TRY_RET_APP_MEMBER(renderer_);
	}

	static AppCanvas& Canvas()
	{
		TRY_RET_APP_MEMBER(canvas_);
	}

	static AppEvents& Events()
	{
		TRY_RET_APP_MEMBER(events_);
	}

	static Status Start(WindowArgs winArgs={})
	{
		auto fail = []() { return Status{ SDL_GetError(), Status::Response::Exit }; };

		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { fail(); }
		if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) { fail(); }
		if (TTF_Init() != 0) { fail(); }

		App::app_ = new App{ {}, {} };

		const auto& winStatus = App::app_->window_.Init(std::move(winArgs));
		if (!winStatus.Good())
		{
			return winStatus;
		}

		const auto& rendStatus = App::app_->renderer_.Init(App::app_->window_);
		if (!rendStatus.Good())
		{
			return rendStatus;
		}

		SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "best", SDL_HINT_OVERRIDE);

		return {};
	}

	static void Exit()
	{
		if (App::app_) 
		{ 
			TTF_Quit();
			IMG_Quit();
			SDL_Quit(); 
		}
	}
};

