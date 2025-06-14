#include"plt/context.h"


#ifdef _WIN32
#include <print>
#include <memory_resource>
#include "system_error.hpp"
#include <assert.h>
#include <mimalloc.h>
    namespace vkd::plt{
        class WinThreadPool : public ThreadPool{

            struct TaskWarrper{
          
                TaskWarrper(std::function<void()>&& task,WinThreadPool* pool) : task_(std::move(task)),pool_(pool) {}
                void run(){
                    task_();
                }
                void destory()
                {
                    pool_->destroyTask(this);
                }

                PTP_WORK work_;
                std::function<void()> task_;
                WinThreadPool * pool_;
                
            };
            
            struct TimerWarrper{
                std::function<void()> task_;
                WinThreadPool * pool_;
                PTP_TIMER timer_;
                std::atomic_uint8_t flag_;
                enum {
                    fInterval = 1,
                    fCanceled = 0b10,
                    fRun = 0b100
                
                };


                 TimerWarrper(std::function<void()>&& task,WinThreadPool* pool,bool isInterval = false) 
                : task_(std::move(task)),pool_(pool),flag_(isInterval?1:0) {}
                void destory(){
                    pool_->destroyTimer(this);
                }

                void cancel(){
                    flag_.fetch_and(fCanceled);
                }

                void run(){
                    if(flag_.load()&fCanceled){
                        return;
                    }
                    task_();
                    flag_.fetch_and(fRun);
                }
            };


           

        public:
            WinThreadPool() {
                InitializeThreadpoolEnvironment(&callBackEnviron_);
                
            };

            void init(int minThreadSize = 1, int maxThreadSize = -1) override{
                if(initialized_) return;

                if(maxThreadSize == -1){
                    maxThreadSize = std::thread::hardware_concurrency();
                }
                assert(minThreadSize > 0);
                
                assert(maxThreadSize > 0);

                pool_ = CreateThreadpool(nullptr);
                ThrowIfFailed(pool_ != nullptr,"CreateThreadPool failed");
                // set min and max thread size
                ThrowIfFailed( SetThreadpoolThreadMinimum(pool_, minThreadSize) == FALSE ,"SetThreadpoolThreadMinimum failed");
                SetThreadpoolThreadMaximum(pool_, maxThreadSize);

                //  Create a cleanup group for this thread pool.
                
                cleanupGroup_ = CreateThreadpoolCleanupGroup();
                ThrowIfFailed(cleanupGroup_ != nullptr,"CreateThreadpoolCleanupGroup failed");

                // set callback

                SetThreadpoolCallbackPool(&callBackEnviron_, pool_);
                SetThreadpoolCallbackCleanupGroup(&callBackEnviron_, cleanupGroup_, nullptr);
                initialized_ = true;
            };
            void stop(bool force) override{
                assert(initialized_);
                if(cleanupGroup_){
                    CloseThreadpoolCleanupGroupMembers(cleanupGroup_, force, nullptr);
                    CloseThreadpoolCleanupGroup(cleanupGroup_);
                    cleanupGroup_ = nullptr;
                }
                if(pool_){
                    CloseThreadpool(pool_);
                    pool_ = nullptr;
                }
            };
           
            void submit( std::function<void()>&& task) override{
                throwIfStoped();
                static auto taskDestroy  = [](TaskWarrper* task){
                    task->~TaskWarrper();
                    task->pool_->taskAllocator_.deallocate(task,1);
                };
                auto taskWarrper =  taskAllocator_.allocate(1);
                std::unique_ptr<TaskWarrper,decltype(taskDestroy)> taskWarrPtr (taskWarrper,taskDestroy);
                new (taskWarrPtr.get()) TaskWarrper {std::move(task),this};

                auto work = CreateThreadpoolWork(WorkCallback,taskWarrPtr.get(),&callBackEnviron_);
                ThrowIfFailed(work!= nullptr,"CreateThreadpoolWork failed");
                taskWarrPtr->work_ = work;
                SubmitThreadpoolWork(taskWarrPtr->work_);
                taskWarrPtr.release();
            };
            LpInterval addInterval( std::function<void()>&& task, int ms) override{
                throwIfStoped();
                static const auto timerDestroy = [](TimerWarrper* task){
                    task->~TimerWarrper();
                    task->pool_->timerAllocator_.deallocate(task,1);
                };

                auto timerWarrper =  timerAllocator_.allocate(1);
                std::unique_ptr<TimerWarrper,decltype(timerDestroy)> timerWarrperPtr (timerWarrper,timerDestroy);
                new (timerWarrperPtr.get()) TimerWarrper {std::move(task),this,false};
                auto Timmer = CreateThreadpoolTimer(TimerCallback,timerWarrperPtr.get(),&callBackEnviron_);

                ThrowIfFailed(Timmer != nullptr,"CreateThreadpoolTimer failed");
                timerWarrperPtr->timer_ = Timmer;
                FILETIME ftDueTime;

                ULARGE_INTEGER  delay ;
                delay.QuadPart = (ULONGLONG) - (10000 * ms);

                ftDueTime.dwLowDateTime = delay.HighPart;
                ftDueTime.dwHighDateTime = delay.LowPart;

                SetThreadpoolTimer(timerWarrperPtr->timer_,&ftDueTime,ms,0);
                return timerWarrperPtr.release();
            };
            void stopInterval(LpInterval interval) override{
                TimerWarrper* timer = (TimerWarrper*)interval;
                timer->destory();
            }
            LpTimer addTimer( std::function<void()>&& task, int ms) override{
                throwIfStoped();
                static const auto timerDestroy = [](TimerWarrper* task){
                    task->~TimerWarrper();
                    task->pool_->timerAllocator_.deallocate(task,1);
                };

                auto timerWarrper =  timerAllocator_.allocate(1);
                std::unique_ptr<TimerWarrper,decltype(timerDestroy)> timerWarrperPtr (timerWarrper,timerDestroy);
                new (timerWarrperPtr.get()) TimerWarrper {std::move(task),this,false};
                auto Timmer = CreateThreadpoolTimer(TimerCallback,timerWarrperPtr.get(),&callBackEnviron_);

                ThrowIfFailed(Timmer != nullptr,"CreateThreadpoolTimer failed");
                timerWarrperPtr->timer_ = Timmer;

                FILETIME ftDueTime;

                ULARGE_INTEGER  delay ;
                delay.QuadPart = (ULONGLONG) - (10000 * ms);;

                ftDueTime.dwLowDateTime = delay.HighPart;
                ftDueTime.dwHighDateTime = delay.LowPart;

                SetThreadpoolTimer(timerWarrperPtr->timer_,&ftDueTime,0,0);
                return timerWarrperPtr.release();
            }

            void cancelTimer(LpTimer timer) override{
                auto timmer = static_cast<TimerWarrper*>(timer);
                timmer->cancel();
            }

            ~WinThreadPool() noexcept override {
                stop(false);
            }

            static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_TIMER timer){
                auto timerWarrper = static_cast<TimerWarrper*>(context);
                try{
                    // not canceled
                    if(timerWarrper->flag_.load() & TimerWarrper::fCanceled == 0)
                        timerWarrper->run();

                }catch(const std::exception& e){
                    std::println(stderr,"Uncaught exception in timer: {}" ,e.what());
                }catch(...){
                    std::println(stderr,"Uncaught exception in timer");
                }
                // not interval
                if(timerWarrper->flag_.load() & TimerWarrper::fInterval == 0){
                    timerWarrper->destory();
                }
            }
            static void CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work) { 
                
                try
                {
                   auto task = static_cast<TaskWarrper*>(context);
                   task->run();
                   task->destory();
                }
                catch(const std::exception& e)
                {
                    std::println(stderr,"Uncaught exception in task: {}" ,e.what());
                }
                catch(...)
                {
                    std::println(stderr,"Uncaught exception in task");
                }
            }
        private:
           void throwIfStoped(){ 
                if(stoppRequested_){
                    throw std::runtime_error("ThreadPool is stopped");
                }
           }
            
          

            void destroyTask(TaskWarrper* task){
                if(task->work_){
                    CloseThreadpoolWork(task->work_);
                    task->work_ = nullptr;
                }

                taskAllocator_.deallocate(task,1); 
            };

          

            void destroyTimer(TimerWarrper* task){ 
                if(task->timer_){
                    SetThreadpoolTimer(task->timer_,nullptr,0,0);
                    CloseThreadpoolTimer(task->timer_);
                    task->timer_ = nullptr;
                }
                timerAllocator_.deallocate(task,1);

            }
        private:

         

            PTP_POOL  pool_ = nullptr;
            PTP_CLEANUP_GROUP  cleanupGroup_ = nullptr;
            TP_CALLBACK_ENVIRON callBackEnviron_;

            std::atomic_bool initialized_ = false;
            std::atomic_bool stoppRequested_ = false;

            mi_stl_allocator<TaskWarrper> taskAllocator_;
            mi_stl_allocator<TimerWarrper> timerAllocator_;
        
            
        };


        __declspec(dllexport) ThreadPool * ThreadPool::CreateWin32ThreadPool(){
            return new WinThreadPool();
        }; 

    }


#endif