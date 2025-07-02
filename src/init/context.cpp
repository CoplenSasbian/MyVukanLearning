#include "context.h"
#include <print>

AppContext::AppContext()
{

	

	
}




void AppContext::run() {
	bool running = true;

	auto uiThreadScheduler = execContext_.getScheduler(vkd::exec::ThreadType::MainThread);
	std::execution::start_detached(
		std::execution::schedule(uiThreadScheduler)
		| std::execution::let_value([&]() {

			return main();
			})
	);

	while (running)
	{
		if (!eventLoop_.pollEvent()) {
			execContext_.poll();
		}

	}

}

exec::task<void> AppContext::main()
{
	window_.create(L"Vkd Application", { 100, 100, 800, 600 });
	eventLoop_.registerWindow(&window_);

	std::println("{}", std::this_thread::get_id());

	co_await exec::reschedule_coroutine_on(execContext_.getScheduler(vkd::exec::ThreadType::ComputeThread));
	std::println("{}", std::this_thread::get_id());

	co_await exec::reschedule_coroutine_on(execContext_.getScheduler(vkd::exec::ThreadType::GraphicsThread));
	std::println("{}", std::this_thread::get_id());

	co_await exec::reschedule_coroutine_on(execContext_.getScheduler(vkd::exec::ThreadType::MainThread));
	std::println("{}", std::this_thread::get_id());

	co_await std::execution::start_on(
		execContext_.getScheduler(vkd::exec::ThreadType::GraphicsThread), 
		std::execution::just(1) | std::execution::then([](int value) {
			std::println("Graphics thread task executed with value: {}", value);
			std::println("{}", std::this_thread::get_id());
		})
	);


	co_return;
}
