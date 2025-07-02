
#pragma once
#include <vkd/context.h>
#include <vkd/window.h>
#include <exec/task.hpp>
class AppContext :public vkd::ImMoveable, vkd::NonCopyable {
public:
	static AppContext& Instance() {
		static AppContext instance;
		return instance;
	}

	void run();

	exec::task<void> main();

private:
	AppContext();

	vkd::exec::Context execContext_;
	vkd::window::Window window_;
	vkd::window::EventLoop eventLoop_;

};