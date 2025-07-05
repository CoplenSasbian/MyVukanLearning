#include "context.h"
#include <print>

AppContext::AppContext()
{




}




void AppContext::run() {

	
	auto uiThreadScheduler = execContext_.getScheduler(vkd::exec::ThreadType::MainThread);

	std::execution::start_detached(
		std::execution::schedule(uiThreadScheduler) |
		std::execution::let_value([&] {return async_run(); })
	);

	while (!quit_)
	{
		if (!eventLoop_.pollEvent()) {
			execContext_.poll();
		}

	}
}

exec::task<void> AppContext::async_run() {
	co_await create_window();
	co_await addEventListener();
	co_await main();
}


exec::task<void> AppContext::create_window()
{
	window_.create(L"Vkd Application", { 100, 100, 800, 600 });
	eventLoop_.registerWindow(&window_);
	co_return;
}

exec::task<void> AppContext::addEventListener()
{
	

	closeListener_ = eventLoop_.on<vkd::window::CloseEvent>(std::execution::then([&](const vkd::window::CloseEvent* e) {
		eventLoop_.postQuitEvent();
	}));

	sizeListener_ = eventLoop_.on<vkd::window::SizeEvent>(std::execution::then([&](const vkd::window::SizeEvent* e) {
		std::println("size: {} {}", e->size().width,e->size().height );
		}));
	quitListener_ = eventLoop_.on<vkd::window::QuitEvent>(std::execution::then([&](const vkd::window::QuitEvent* e) {
		quit_ = true;
	}));

	clickListener_ = eventLoop_.on<vkd::window::MouseEvent>(std::execution::then([&](const vkd::window::MouseEvent* e) {
		static bool on = true;
		
		if (e->button() == e->MB_R && e->pressed()) {
			on = !on;
			if (on) {
				eventLoop_.resumeListener(sizeListener_);
			}else{
				eventLoop_.pauseListener(sizeListener_);
			}
		}
		
	}));

	co_return;
}

exec::task<void> AppContext::main()
{
	std::println("Main fun");


	

	co_return;
}
