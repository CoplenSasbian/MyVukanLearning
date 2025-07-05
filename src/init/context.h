
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

protected:


	exec::task<void> async_run();

	exec::task<void> create_window();
	
	exec::task<void> addEventListener();

	exec::task<void> main();

private:
	AppContext();
	bool quit_ = false;
	vkd::exec::Context execContext_;
	vkd::window::Window window_;
	vkd::window::EventLoop eventLoop_;

};