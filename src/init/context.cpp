#include "context.h"
#include <spdlog/spdlog.h>
#include "spdlog/async.h" 
#include "spdlog/sinks/basic_file_sink.h"
#include <spdlog/sinks/wincolor_sink.h>
AppContext::AppContext()
{




}




void AppContext::run() {

	spdlog::init_thread_pool(8192, 2); 

#ifdef WIN32
	auto stdout_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#endif // WIN32
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>("logs/log.txt");

	std::vector<spdlog::sink_ptr> sinks{ stdout_sink, file_sink };
	auto logger = std::make_shared<spdlog::async_logger>("multi_sink", sinks.begin(), sinks.end(), spdlog::thread_pool());
	spdlog::register_logger(logger);
	spdlog::set_default_logger(logger);

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
	spdlog::drop_all();

}

AppContext::~AppContext()
{
}

exec::task<void> AppContext::async_run() {

	try {
		co_await create_window();
		co_await addEventListener();
		co_await main();
	}
	catch (const std::exception& e) {
		spdlog::error("uncouth exception:{}", e.what());
	}
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
		spdlog::info("On window close!");
		eventLoop_.postQuitEvent();
		
	}));

	sizeListener_ = eventLoop_.on<vkd::window::SizeEvent>(std::execution::then([&](const vkd::window::SizeEvent* e) {
		spdlog::info("On window resize: {} {}.", e->size().x, e->size().y);
		}));
	quitListener_ = eventLoop_.on<vkd::window::QuitEvent>(std::execution::then([&](const vkd::window::QuitEvent* e) {
		spdlog::info("Required quit!");
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


	

	co_return;
}
