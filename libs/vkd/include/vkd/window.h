#pragma once

#include <vkd/core.h>
#include <string>
#include <memory>
namespace vkd::window {

	struct Rect {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		Rect() = default;
		Rect(int x, int y, int width, int height)
			: x(x), y(y), width(width), height(height) {
		}
		template<typename T>
		Rect(const T& rect){
			x = std::get<0>(rect);
			y = std::get<1>(rect);
			width = std::get<2>(rect);
			height = std::get<3>(rect);
		}
		bool operator==(const Rect& other) const {
			return x == other.x && y == other.y && width == other.width && height == other.height;
		}
	};

	struct Vec2{
		int x = 0;
		int y = 0;
		
		Vec2() = default;
		Vec2(int x, int y)
			: x(x), y(y) {
		}
		template<typename T>
		Vec2(const T& vec) {
			x = std::get<0>(vec);
			y = std::get<1>(vec);
		}

		bool operator==(const Vec2& other) const {
			return x == other.x && y == other.y;
		}

	} Size,Point;

	class Window : public vkd::NonCopyable {
	public:
		DLL_API Window();
		DLL_API ~Window();
		DLL_API void create(
			const std::wstring& title, 
			const Rect& rect= {-1,-1,800,600 });
		DLL_API void show();
		DLL_API void hide();
		DLL_API void close();


	private:
		struct Impl;
		std::unique_ptr<Impl> impl_;
	};
} // namespace vkd::window