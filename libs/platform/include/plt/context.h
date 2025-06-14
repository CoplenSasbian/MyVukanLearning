#pragma once
#include "base.h"

#include <functional>
#include <future>
#include <type_traits>

namespace vkd::plt{
  using LpTimer = void*;  
  using LpInterval = void*;
  class ThreadPool{

    public:
      
    
      virtual ~ThreadPool() = default;
    
      virtual void init(int minThreadSize = 1, int maxThreadSize = -1) = 0;

      virtual void submit( std::function<void()>&& task) = 0;
      
      virtual LpTimer addTimer( std::function<void()>&& task, int ms) = 0;

      virtual void cancelTimer(LpTimer timer) = 0;

      virtual LpInterval addInterval( std::function<void()>&& task, int ms) = 0;

      virtual void stopInterval(LpInterval interval) = 0;

      virtual void stop(bool force = false) = 0;


      template<class F, class ...Args>
      auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F(Args...)>::type> {
        using return_type = typename std::invoke_result_t<F(Args...)>::type;

        auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...) );
        std::future<return_type> res = task->get_future();
        submit([task](){ (*task)(); });
        return res;
      }




      #if  defined(_WIN32)
       PLT_API static ThreadPool* CreateWin32ThreadPool();
      #endif

      PLT_API static ThreadPool* CreateStdThreadPool();

    };


    
}