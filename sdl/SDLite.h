#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <vector>
#include <cassert>
#include "SDLUtils.h"

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

		typename AppPtrT::PtrType appPtr_;
	};

	static constexpr int kWindowWidth = 1200;
	static constexpr int kWindowHeight = 900;
	static constexpr SDL_Point kWindowCenter = { 
		kWindowWidth / 2, kWindowHeight / 2 
	};
	static constexpr SDL_FPoint kFWindowCenter = { 
		static_cast<float>(kWindowWidth / 2.0f), static_cast<float>(kWindowHeight / 2.0f) 
	};

	struct WindowArgs
	{
		static constexpr int kScreenPaddingX = 40;
		static constexpr int kScreenPaddingY = 60;

		int w = 0;
		int h = 0;
		int x = SDL_WINDOWPOS_UNDEFINED;
		int y = SDL_WINDOWPOS_UNDEFINED;
		std::string title;
		Uint32 flags = (
			SDL_WINDOW_SHOWN |
			SDL_WINDOW_RESIZABLE |
			SDL_WINDOW_ALLOW_HIGHDPI
		);
	};

	using WindowPtr = AppPtr<SDL_Window, WindowArgs,
		[](WindowArgs wa) { return SDL_CreateWindow(wa.title.c_str(), wa.x, wa.y, wa.w, wa.h, wa.flags); },
		[](SDL_Window* win) { SDL_DestroyWindow(win); }
	>;

	class AppWindow : public AppObject<WindowPtr>
	{
	public:
		friend Status Start(WindowArgs);

		template <IntOrFloat T = int>
		Dimensions<T> GetSize() const;
		template <IntOrFloat T = int>
		void SetSize(Dimensions<T> dims);

		template <SDLPointType P = SDL_Point>
		P GetPosition() const;
		template <SDLPointType P = SDL_Point>
		void SetPosition(P p);

		template <SDLPointType P = SDL_Point>
		P GetLocalCenter() const;

		void Minimize() { SDL_MinimizeWindow(appPtr_.get()); }
		void Maximize() { SDL_MaximizeWindow(appPtr_.get()); }
		Uint32 GetFlags() const { return SDL_GetWindowFlags(appPtr_.get()); }
		bool IsMinimized() const { return GetFlags() & SDL_WINDOW_MINIMIZED; }
		bool IsMaximized() const { return GetFlags() & SDL_WINDOW_MAXIMIZED; }
		void Restore() { SDL_RestoreWindow(appPtr_.get()); }
		//SDL_FPoint GetDpiScaling() const;

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

	class App
	{
	public:
		friend Status Start(WindowArgs);
		friend void Exit();
		friend bool Running();

		friend AppWindow& Window();
		friend AppRenderer& Renderer();

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

	static Status Start(WindowArgs winArgs={})
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) 
		{ 
			return Status{ SDL_GetError(), Status::Response::Exit };
		}
		if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) 
		{ 
			return Status{ SDL_GetError(), Status::Response::Exit };
		}
		if (TTF_Init() != 0) 
		{ 
			return Status{ TTF_GetError(), Status::Response::Exit };
		}
	
		int mixFlags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
		int mixInit = Mix_Init(mixFlags);
		if ((mixInit & mixFlags) != mixFlags) 
		{
			return Status{ Mix_GetError(), Status::Response::Exit };
		}
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
			return Status{ Mix_GetError(), Status::Response::Exit };
		}

		App::app_ = new App{ {}, {} };

		if (winArgs.w <= 0 || winArgs.h <= 0)
		{
			SDL_DisplayMode mode;
			if (SDL_GetDesktopDisplayMode(0, &mode) == 0)
			{
				winArgs.w = mode.w - WindowArgs::kScreenPaddingX;
				winArgs.h = mode.h - WindowArgs::kScreenPaddingY;
			}
			else
			{
				winArgs.w = kWindowWidth;
				winArgs.h = kWindowHeight;
			}
		}
		const auto& winStatus = App::app_->window_.Init(std::move(winArgs));
		if (!winStatus.Good())
		{
			return winStatus;
		}

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

		const auto& rendStatus = App::app_->renderer_.Init(App::app_->window_);
		if (!rendStatus.Good())
		{
			return rendStatus;
		}

		//SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "best", SDL_HINT_OVERRIDE);

		return {};
	}

	static void Exit()
	{
		if (App::app_) 
		{ 
			Mix_CloseAudio();
			Mix_Quit();
			TTF_Quit();
			IMG_Quit();
			SDL_Quit(); 
		}
	}

	static bool Running()
	{
		return App::app_ != nullptr;
	}

	//SDL_FPoint AppWindow::GetDpiScaling() const
	//{
	//	if (!Renderer())
	//	{
	//		return { 1.0f, 1.0f };
	//	}

	//	int logicalW, logicalH;
	//	SDL_GetWindowSize(appPtr_.get(), &logicalW, &logicalH);
	//	if (logicalW == 0 || logicalH == 0)
	//	{
	//		return { 1.0f, 1.0f };
	//	}

	//	int pixelW, pixelH;
	//	SDL_GetRendererOutputSize(Renderer(), &pixelW, &pixelH);

	//	return {
	//		static_cast<float>(pixelW) / static_cast<float>(logicalW),
	//		static_cast<float>(pixelH) / static_cast<float>(logicalH),
	//	};
	//}

	template <IntOrFloat T>
	Dimensions<T> AppWindow::GetSize() const
	{
		int w, h;
		SDL_GetWindowSize(appPtr_.get(), &w, &h);

		if (Renderer())
		{
			SDL_GetRendererOutputSize(Renderer(), &w, &h);
		}

		return Dimensions<T>{ static_cast<T>(w), static_cast<T>(h) };
	}
	template <IntOrFloat T>
	void AppWindow::SetSize(Dimensions<T> dims)
	{
		SDL_SetWindowSize(appPtr_.get(), static_cast<int>(dims.w),
										 static_cast<int>(dims.h));
	}

	template <SDLPointType P>
	P AppWindow::GetPosition() const
	{
		using ValueType = std::remove_cvref_t<decltype(P::x)>;
		int x, y;
		SDL_GetWindowPosition(appPtr_.get(), &x, &y);

		return P{ static_cast<ValueType>(x), static_cast<ValueType>(y) };
	}
	template <SDLPointType P>
	void AppWindow::SetPosition(P p)
	{
		SDL_SetWindowPosition(appPtr_.get(), static_cast<int>(p.x),
											 static_cast<int>(p.y));
	}

	template <SDLPointType P>
	P AppWindow::GetLocalCenter() const
	{
		using ValueType = std::remove_cvref_t<decltype(P::x)>;
		auto dims = GetSize<ValueType>();

		return P{
			static_cast<ValueType>(dims.w / 2.0f),
			static_cast<ValueType>(dims.h / 2.0f)
		};
	}
};

