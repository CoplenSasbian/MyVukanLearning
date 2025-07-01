#pragma once
#ifdef WIN32
#include <vkd/window.h>
#include <Windows.h>
#include <mutex>
#include <cassert>

namespace vkd::window{

	struct Window::Impl
	{
		HWND hWnd_;
	};
	
	constexpr auto WindowClassName = L"VkdWindowClass";

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if(WM_CREATE == message) {
			CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* window = static_cast<Window*>(createStruct->lpCreateParams);
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	};


	static void RegisterWindowClass() {
		static std::once_flag onceFlag;
	
		std::call_once(onceFlag, []() {
			WNDCLASSW wc = {};
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandleW(nullptr);
			wc.lpszClassName = WindowClassName;
			if (!RegisterClassW(&wc)) {
				throw std::runtime_error("Failed to register window class");
			}
		});

		}
	}


	vkd::window::Window::Window()
	{
		impl_ = std::make_unique<Impl>();
		RegisterWindowClass();
	}

	 vkd::window::Window::~Window(){
		if (impl_->hWnd_) {
			DestroyWindow(impl_->hWnd_);
		}
	 }

	void vkd::window::Window::create(const std::wstring& title, const Rect& rect){
		
		int x = rect.x == -1 ? CW_USEDEFAULT : rect.x;
		int y = rect.y == -1 ? CW_USEDEFAULT : rect.y;
		int width = rect.width == -1 ? CW_USEDEFAULT : rect.width;
		int height = rect.height == -1 ? CW_USEDEFAULT : rect.height;

		assert(x > 0 && y > 0 && width > 200 && height > 200 && "Create rect error");

		impl_->hWnd_ = CreateWindowExW(
			0, WindowClassName, title.c_str(),
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			x, y, width, height,
			NULL, NULL, GetModuleHandleW(nullptr), this);
		if (!impl_->hWnd_) {
			throw std::runtime_error("Failed to create window");
		}
		
	}

	 void vkd::window::Window::show()
	 {
		 
	 }

	 void vkd::window::Window::hide()
	 {
		 
	 }

	  void vkd::window::Window::close()
	 {
		 
	 }

	  
}



#endif // WIN32
