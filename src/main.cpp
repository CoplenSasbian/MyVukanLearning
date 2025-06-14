#include<print>
#include<plt/platform.h>
#include<plt/context.h>
// #include<mimalloc.h>
#include<iostream>
auto main()->int{
    
    // mi_process_init();

    std::println("hello {}",vkd::plt::libVersion());

    
    auto pool = vkd::plt::ThreadPool::CreateWin32ThreadPool();

    pool->init();

    pool->submit([]{
        std::println("hello from thread");
    });

    pool->addTimer([]{
        std::println("hello from timer");
    },1000);
    
    auto pi= pool->addInterval([]{
        std::println("hello from interval");
    },1000);


    auto ai = pool->addTimer([]{
        std::println("hello from timer Will not be call");
    },1000);

    pool->cancelTimer(ai);

    // std::getchar();
    std::this_thread::sleep_for(std::chrono::seconds(4));
    pool->stopInterval(pi);

    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::println("request stop");
    pool->stop();
}