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
	auto winClose = eventLoop_.on<vkd::window::CloseEvent>() |
		std::execution::then([&](const vkd::window::CloseEvent* e) {
		eventLoop_.postQuitEvent();
		});

	auto quitListener = eventLoop_.on<vkd::window::QuitEvent>() |
		std::execution::then([&](const vkd::window::QuitEvent* e) {
		quit_ = true;
		});

	auto sizeChangeListener = eventLoop_.on<vkd::window::SizeEvent>() |
		std::execution::then([&](const vkd::window::SizeEvent* e) {
		std::println("resize :{} {}", e->size().x, e->size().y);
		});

	std::execution::start_detached(winClose);
	std::execution::start_detached(quitListener);
	std::execution::start_detached(sizeChangeListener);
	co_return;
}

exec::task<void> AppContext::main()
{
	std::println("Main fun");


	

	co_return;
}
