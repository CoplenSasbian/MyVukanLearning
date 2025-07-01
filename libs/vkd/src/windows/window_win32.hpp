#pragma once
#ifdef WIN32
#include <vkd/window.h>
#include <Windows.h>
#include <windowsx.h>
#include <mutex>
#include <cassert>

namespace vkd::window {



	constexpr auto WindowClassName = L"VkdWindowClass";

	constexpr intptr_t WndClassOffset = GWLP_USERDATA;

	constexpr intptr_t EventLoopOffset = GWLP_USERDATA + sizeof(void*);
	struct NativeEvent :MSG
	{
	};


	LRESULT transformEvent(const NativeEvent& e) {
		std::unique_ptr<Event> event;

		auto pel = (EventLoop*)::GetWindowLongPtrW(e.hwnd, EventLoopOffset);

		switch (e.message) {
		case WM_CLOSE:
			event = std::make_unique<CloseEvent>(e);
			break;

		case WM_SIZE:
			event = std::make_unique<SizeEvent>(e);
			break;

		case WM_MOVE:
			event = std::make_unique<MoveEvent>(e);
			break;

		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			event = std::make_unique<FocusEvent>(e);
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			event = std::make_unique<KeyEvent>(e);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
			event = std::make_unique<MouseEvent>(e);
			break;

		case WM_MOUSEWHEEL:
			event = std::make_unique<WheelEvent>(e);
			break;

		case WM_PAINT:
			event = std::make_unique<PaintEvent>(e);
			break;

		case WM_SHOWWINDOW:
			event = std::make_unique<ShowEvent>(e);
			break;

		case WM_WINDOWPOSCHANGED: {
			WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(e.lParam);
			if (pos->flags & SWP_HIDEWINDOW) {
				event = std::make_unique<IconifyEvent>(e);
			}
			break;
		}

		case WM_DROPFILES:
			event = std::make_unique<FileDropEvent>(e);
			break;

		default:
			return DefWindowProcW(e.hwnd, e.message, e.wParam, e.lParam);
		}

		if (event) {
			pel->setEvent(event.get());

			if (!event->hasPreventDefault()) {
				return DefWindowProcW(e.hwnd, e.message, e.wParam, e.lParam);
			}
		}

		return  0;
	}

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (WM_CREATE == message) {
			CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* window = static_cast<Window*>(createStruct->lpCreateParams);
			SetWindowLongPtrW(hWnd, WndClassOffset, reinterpret_cast<LONG_PTR>(window));
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		NativeEvent e;
		e.hwnd = hWnd;
		e.message = message;
		e.wParam = wParam;
		e.lParam = lParam;
		return transformEvent(e);
	};


	static void RegisterWindowClass() {
		static std::once_flag onceFlag;

		std::call_once(onceFlag, []() {
			WNDCLASSEXW wc = {};
			wc.cbSize = sizeof(WNDCLASSEXW);
			wc.cbWndExtra = 8 * sizeof(intptr_t);
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandleW(nullptr);
			wc.lpszClassName = WindowClassName;
			if (!RegisterClassExW(&wc)) {
				throw std::runtime_error("Failed to register window class");
			}
			});


	}

	struct Window::Impl
	{
		HWND hWnd_;
	};

	Window::Window() {
		impl_ = std::make_unique<Impl>();
		RegisterWindowClass();
	}

	Window::~Window() {
		if (impl_->hWnd_) {
			DestroyWindow(impl_->hWnd_);
		}
	}

	void Window::create(const std::wstring& title, const Rect& rect) {

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
	void* Window::nativeHandel()const {
		return impl_->hWnd_;
	}

	static int toWin32Show(WindowShow ws) {

		int nSw = SW_NORMAL;
		switch (ws)
		{
		case vkd::window::WS_SHOW:
			nSw = SW_NORMAL;
			break;
		case vkd::window::WS_HIDE:
			nSw = SW_HIDE;
			break;
		case vkd::window::WS_MAXSIZE:
			nSw = SW_MAXIMIZE;
			break;
		case vkd::window::WS_MINISIZE:
			nSw = SW_MINIMIZE;
			break;
		}
		return nSw;
	}

	void Window::show(WindowShow ws)const {

		::ShowWindow(impl_->hWnd_, toWin32Show(ws));
	}


	void Window::close()const {
		::CloseWindow(impl_->hWnd_);
	}


	bool EventLoop::pollEvent() const
	{
		static NativeEvent event{};

		if (::PeekMessageW(&event, 0, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&event);
			::DispatchMessageW(&event);
			return true;
		}
		return false;
	}



	Event::Event(const NativeEvent& msg) {
		window_ = reinterpret_cast<Window*>(GetWindowLongPtrW(msg.hwnd, WndClassOffset));
	}


	SizeEvent::SizeEvent(const NativeEvent& msg) : Event(msg) {
		size_.width = LOWORD(msg.lParam);
		size_.height = HIWORD(msg.lParam);
	}

	MoveEvent::MoveEvent(const NativeEvent& msg) : Event(msg) {
		position_.x = LOWORD(msg.lParam);
		position_.y = HIWORD(msg.lParam);
	}

	FocusEvent::FocusEvent(const NativeEvent& msg) : Event(msg) {
		gained_ = (msg.wParam == WA_ACTIVE || msg.wParam == WA_CLICKACTIVE);
	}

	KeyEvent::KeyEvent(const NativeEvent& msg) : Event(msg) {
		keyCode_ = static_cast<uint16_t>(msg.wParam);
		pressed_ = (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN);
		repeated_ = (msg.lParam & (1 << 30)) != 0;

		modifiers_ = MOD_NONE;
		if (GetKeyState(VK_MENU) & 0x8000) modifiers_ |= MOD_ALT;
		if (GetKeyState(VK_CONTROL) & 0x8000) modifiers_ |= MOD_CTRL;
		if (GetKeyState(VK_SHIFT) & 0x8000) modifiers_ |= MOD_SHIFT;
		if (GetKeyState(VK_LWIN) & 0x8000 || GetKeyState(VK_RWIN) & 0x8000) modifiers_ |= MOD_WIN;
		if (GetKeyState(VK_CAPITAL) & 1) modifiers_ |= MOD_CAPS;
		if (GetKeyState(VK_NUMLOCK) & 1) modifiers_ |= MOD_NUM;
	}

	MouseEvent::MouseEvent(const NativeEvent& msg) : Event(msg) {
		position_.x = GET_X_LPARAM(msg.lParam);
		position_.y = GET_Y_LPARAM(msg.lParam);

		switch (msg.message) {
		case WM_LBUTTONDOWN: case WM_LBUTTONUP:
			button_ = MB_L; break;
		case WM_RBUTTONDOWN: case WM_RBUTTONUP:
			button_ = MB_R; break;
		case WM_MBUTTONDOWN: case WM_MBUTTONUP:
			button_ = MB_M; break;
		}

		pressed_ = (msg.message == WM_LBUTTONDOWN ||
			msg.message == WM_RBUTTONDOWN ||
			msg.message == WM_MBUTTONDOWN);
	}

	WheelEvent::WheelEvent(const NativeEvent& msg) : Event(msg) {
		delta_ = GET_WHEEL_DELTA_WPARAM(msg.wParam);
		position_ = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
	}

	PaintEvent::PaintEvent(const NativeEvent& msg) : Event(msg) {
		if (msg.wParam) {
			RECT rc;
			GetUpdateRect(msg.hwnd, &rc, FALSE);
			region_ = { rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top };
		}
		else {

			RECT rc;
			GetClientRect(msg.hwnd, &rc);
			region_ = { 0, 0, rc.right, rc.bottom };
		}
	}

	ShowEvent::ShowEvent(const NativeEvent& msg) : Event(msg) {
		shown_ = (msg.wParam != 0);
	}

	IconifyEvent::IconifyEvent(const NativeEvent& msg) : Event(msg) {
		iconified_ = (msg.wParam == SIZE_MINIMIZED);
	}

	FileDropEvent::FileDropEvent(const NativeEvent& msg) : Event(msg) {
		HDROP hDrop = reinterpret_cast<HDROP>(msg.wParam);
		position_.x = GET_X_LPARAM(msg.lParam);
		position_.y = GET_Y_LPARAM(msg.lParam);

		UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
		for (UINT i = 0; i < fileCount; ++i) {
			wchar_t path[MAX_PATH];
			DragQueryFileW(hDrop, i, path, MAX_PATH);
			files_.push_back(path);
		}
		DragFinish(hDrop);
	}

	void EventLoop::registerWindow(Window* win)const {
		auto hWnd = (HWND)win->nativeHandel();
		::SetWindowLongPtrW(hWnd, EventLoopOffset, (intptr_t)this);

	}
	void  EventLoop::addListener(EventListener* listener) {
		std::unique_lock lock{ mutex_ };
		listeners_[listener->type_].push_back(listener);
	}

	void EventLoop::setEvent(const Event* e) {
		std::shared_lock lock{ mutex_ };

		auto found = listeners_.find(e->type());
		if (found != listeners_.end()) {
			return;
		}
		auto copy =  found->second;
		lock.unlock();

		for (auto listener : copy)
		{
			listener->start_(listener);
		}
	}


	 


	  
}



#endif // WIN32
