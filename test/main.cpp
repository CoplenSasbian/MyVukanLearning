
#include <print>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>

namespace vkd::exec
{





	namespace __detail {
		class SystemMessageLoop;
		using loop = SystemMessageLoop;
		class SystemMessageScheduler;
		using Scheduler = SystemMessageScheduler;
		struct ImMoveable {
			ImMoveable() = default;
			~ImMoveable() = default;
		private:
			ImMoveable(const ImMoveable&) = delete;
			ImMoveable(ImMoveable&&) = delete;
			ImMoveable& operator=(const ImMoveable&) = delete;
			ImMoveable& operator=(ImMoveable&&) = delete;
		};

		struct Task :public ImMoveable{
			Task* next_ = nullptr;
			
			union 
			{
				Task* tail_ = nullptr;

				void (*execute_)(Task*);
			};

		};

	


		class SystemMessageLoopEnv {
		public:
			SystemMessageLoopEnv(const loop* _loop)
				:loop_(_loop) {
			}

			bool operator == (SystemMessageLoopEnv & rhs)
			{
				return rhs.loop_ == loop_;
			}
			friend Scheduler tag_invoke(stdexec::get_scheduler_t, const SystemMessageLoopEnv& self) noexcept;

			friend Scheduler tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, const SystemMessageLoopEnv& self) noexcept;
		private:
			
			const loop* loop_;
		};

		template<stdexec::receiver R>
		class SystemMessageLoopOptionState:public Task {
			using Env = SystemMessageLoopEnv;
		public:
			SystemMessageLoopOptionState(const loop* _loop,R && r)
				:receiver_(std::forward<R>(r)), loop_(const_cast<loop*>(_loop)){
				Task::execute_ = &SystemMessageLoopOptionState::execute_;
			}

			static void execute_(Task* task) noexcept {
				auto* self = static_cast<SystemMessageLoopOptionState*>(task);
				try{
				
					if (stdexec::get_stop_token(stdexec::get_env(self->receiver_)).stop_requested()) {
						stdexec::set_stopped(std::move(self->receiver_));
						return;
					}
					stdexec::set_value(std::move(self->receiver_));
				}catch (...){
					stdexec::set_error(self->receiver_, std::current_exception());
				}
			}

			void start() const noexcept {
				loop_->__push_back_((Task*)this);
			}

			friend void tag_invoke(stdexec::start_t,const SystemMessageLoopOptionState& self) noexcept {
				self.start();
			}

			
		private:
			R receiver_;
			loop* loop_;
		};


		class SystemMessageLoopSender {
			using Env = SystemMessageLoopEnv;

		public:
			using sender_concept = stdexec::sender_t;

		using completion_signatures = stdexec::completion_signatures<
					stdexec::set_value_t(),
					stdexec::set_error_t(std::exception_ptr),
					stdexec::set_stopped_t()>;

			SystemMessageLoopSender(const loop* _loop)
				:loop_(_loop) {
			}
			SystemMessageLoopSender(SystemMessageLoopSender&& r) noexcept {
				loop_ = std::exchange(r.loop_, nullptr);
			}

			SystemMessageLoopSender(const SystemMessageLoopSender& r)
				:SystemMessageLoopSender(r.loop_) {
			}
			~SystemMessageLoopSender()noexcept
			{}

			
			Env get_env() const{
				return Env{ this->loop_ };
			}

			friend  Env tag_invoke(stdexec::get_env_t, const SystemMessageLoopSender& self)noexcept {
				return self.get_env();
			}

			template<stdexec::receiver R>
			friend SystemMessageLoopOptionState<R> tag_invoke(stdexec::connect_t, const SystemMessageLoopSender& self, R&& r) {
				return SystemMessageLoopOptionState<R>{self.loop_, std::forward<R>(r)};
			}

			 

		private:
			const loop*  loop_;
		};


		class SystemMessageScheduler {
			
			using Sender = SystemMessageLoopSender;
		public:


			SystemMessageScheduler(const loop* _loop)
				:loop_(_loop)
			{}

		

			friend Sender schedule(const SystemMessageScheduler& self) {
				return { self.loop_ };
			}

			
			friend Sender tag_invoke(stdexec::schedule_t, const SystemMessageScheduler& self) {
				return schedule(self);
			}
			
			bool operator == (const SystemMessageScheduler& rhs) const noexcept {
				return rhs.loop_ == loop_;
			}


			

		private:
			const loop* loop_;

			static_assert(stdexec::sender<Sender>);
		};


		class SystemMessageLoop  {

			using Scheduler = SystemMessageScheduler;
		public:
			SystemMessageLoop() = default;

			auto get_scheduler()const noexcept  {
				return Scheduler{this};
			}

			friend Scheduler tag_invoke(stdexec::get_scheduler_t, const SystemMessageLoop& self) noexcept {
				return self.get_scheduler();
			}


			void run() {
				for (Task* task; (task = __pop_front_()) != &head_;) {
					task->execute_(task);
				}
			};


			void poll() {
				while (Task* task = __try_pop_front_()) {
					task->execute_(task);
				}
			};

			void finish() {
				stop_ = true;
				std::unique_lock lock{ mutex_ };
				cv_.notify_all();
			};


		

		private:
			template<stdexec::receiver R>
			friend class SystemMessageLoopOptionState;

			Task* __pop_front_() {
				std::unique_lock lock{ mutex_ };
				cv_.wait(lock, [this] { return head_.next_ != &head_ || stop_; });
				if (head_.tail_ == head_.next_)
					head_.tail_ = &head_;
				return std::exchange(head_.next_, head_.next_->next_);
			}

			Task* __try_pop_front_() {
				std::unique_lock __lock{ mutex_ };
				if (head_.next_ == &head_) {
					return nullptr;
				}
				if (head_.tail_ == head_.next_)
					head_.tail_ = &head_;
				return std::exchange(head_.next_, head_.next_->next_);
			}

			void  __push_back_(Task* task) {
				std::unique_lock __lock{ mutex_ };
				task->next_ = &head_;
				head_.tail_ = head_.tail_->next_ = task;
				cv_.notify_one();
			}


			std::mutex mutex_;
			std::condition_variable cv_;

			Task head_{ {}, &head_,{&head_} };

			bool stop_ = false;
		};


		Scheduler tag_invoke(stdexec::get_scheduler_t, const SystemMessageLoopEnv& self) noexcept
		{
			return self.loop_->get_scheduler();
		}

		Scheduler tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, const SystemMessageLoopEnv& self) noexcept
		{
			return self.loop_->get_scheduler();
		}

}
	using loop = vkd::exec::__detail::SystemMessageLoop;

}



auto main() -> int {


	auto loop = vkd::exec::loop{};



	stdexec::scheduler auto scheduler = stdexec::get_scheduler(loop);

	struct simple_receiver:stdexec::receiver_t {
		void set_value() const noexcept {}
		void set_error(std::exception_ptr) const noexcept {}
		void set_stopped() const noexcept {}
	};

	stdexec::receiver auto simple_rcvr = simple_receiver{};

	auto task = 
		stdexec::start_on(scheduler, stdexec::just() 
			| stdexec::then([]() {
				printf("Hello from the scheduler!\n");
			})
			);

	auto task2 = stdexec::connect(std::move(task), std::move(simple_rcvr));

	stdexec::start(task2);



	loop.poll();
	

}