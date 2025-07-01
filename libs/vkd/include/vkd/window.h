#pragma once

#include "core.h"
#include "execution/stdexec.h"
#include <string>
#include <memory>
#include <vector>
#include <type_traits>
#include <typeindex>
#include <shared_mutex>
namespace vkd::window {

	struct Rect {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		Rect() = default;
		Rect(int x, int y, int width, int height)
			: x(x), y(y), width(width), height(height) {
		}
		template<typename T>
		Rect(const T& rect){
			x = std::get<0>(rect);
			y = std::get<1>(rect);
			width = std::get<2>(rect);
			height = std::get<3>(rect);
		}
		bool operator==(const Rect& other) const {
			return x == other.x && y == other.y && width == other.width && height == other.height;
		}
	};

	typedef struct Vec2{
        union 
        {
            int x;
            int width;
        };
        union 
        {
            int y ;
            int height;

        };
		Vec2() = default;
		Vec2(int x, int y)
			: x(x), y(y) {
		}
		template<typename T>
		Vec2(const T& vec) {
			x = std::get<0>(vec);
			y = std::get<1>(vec);
		}

		bool operator==(const Vec2& other) const {
			return x == other.x && y == other.y;
		}

	} Size,Point;

    enum WindowShow
    {
        WS_SHOW,
        WS_HIDE,
        WS_MAXSIZE,
        WS_MINISIZE

    };

	class Window : public vkd::NonCopyable {
	public:
		DLL_API Window();
		DLL_API ~Window();
		DLL_API void create(
			const std::wstring& title, 
			const Rect& rect= {-1,-1,800,600 });
		DLL_API void show(WindowShow ws) const;
		DLL_API void close() const;
        DLL_API void* nativeHandel() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl_;
	};

	struct NativeEvent;

	class Event {
	public:
		Window* window()
		{
			return window_;
		}
		
        void preventDefault() {
            preventDefault_ = true;
        }
        bool hasPreventDefault() const{
            return preventDefault_;
        }
        std::type_index type() const{
            return typeid (*this);
        }
        
        virtual ~Event(){}
    protected:
        Event(const NativeEvent&);
		friend class EventLoop;
		Window* window_;
        bool preventDefault_ = false;
	};
	class CloseEvent:public Event {
		friend class EventLoop;
		using Event::Event;
	};


	class SizeEvent :public Event {
		friend class EventLoop;
	public:
		const Size& rect()const {
			return size_;
		};
    protected:
        SizeEvent(const NativeEvent&);
		Size size_;
	};

	class MoveEvent : public Event {
		friend class EventLoop;
	public:
		const Point& position() const {
			return position_;
		}
    protected:
        MoveEvent(const NativeEvent&);
		Point position_; 
	};

	class FocusEvent : public Event {
		friend class EventLoop;
	public:
		bool gained() const {
			return gained_;
		}
    protected:
        FocusEvent(const NativeEvent&);
		bool gained_:1;  
	};

    class KeyEvent : public Event{
        friend class EventLoop;
    public:
        enum Modifiers:uint8_t {
            MOD_NONE = 0,
            MOD_ALT = 1 << 0,  // Alt
            MOD_CTRL = 1 << 1,  // Ctrl
            MOD_SHIFT = 1 << 2,  // Shift
            MOD_WIN = 1 << 3,  // Windows 
            MOD_CAPS = 1 << 4,  // Caps Lock
            MOD_NUM = 1 << 5,  // Num Lock
            MOD_ALT_R = 1 << 6,  
            MOD_CTRL_R = 1 << 7   
        };
        int keyCode() const {
            return keyCode_;
        }

        bool pressed() const {
            return pressed_;
        }

        bool repeated() const {
            return repeated_;
        }

        uint8_t modifiers() const {
            return modifiers_;
        } 
    protected:
        KeyEvent(const NativeEvent&);

        uint16_t  keyCode_;
        uint8_t modifiers_;
        bool pressed_ :1;  
        bool repeated_:1; 
    };

    class MouseEvent : public Event {
        friend class EventLoop;
    public:
        enum MouseButton:uint8_t{
            MB_R,
            MB_L,
            MB_M
        };
        const Point& position() const {
            return position_;
        }

        MouseButton button() const {
            return button_;
        }

        bool pressed() const {
            return pressed_;
        }

    protected:
        MouseEvent(const NativeEvent&);
        Point position_;   
        MouseButton button_:4; 
        bool pressed_:1;      
    };

    class WheelEvent : public Event {
        friend class EventLoop;
    public:
        int delta() const {
            return delta_;
        } 

        const Point& position() const {
            return position_;
        }  

    protected:
        WheelEvent(const NativeEvent&);
        int delta_;     
        Point position_;
    };

    class PaintEvent : public Event {
        friend class EventLoop;
    public:
        const Rect& region() const {
            return region_;
        }  

    protected:
        PaintEvent(const NativeEvent&);
        Rect region_;  
    };

    class ShowEvent : public Event {
        friend class EventLoop;
    public:
        bool shown() const {
            return shown_;
        }  

    protected:
        ShowEvent(const NativeEvent&);
        bool shown_:1;
    };

    class IconifyEvent : public Event {
        friend class EventLoop;
    public:
        bool iconified() const {
            return iconified_;
        }  

    protected:
        IconifyEvent(const NativeEvent&);
        bool iconified_:1;
    };

    class FileDropEvent : public Event {
        friend class EventLoop;
    public:
        const std::vector<std::wstring>& files() const {
            return files_;
        }  

        const Point& dropPosition() const {
            return position_;
        } 

    protected:
        FileDropEvent(const NativeEvent&);
        std::vector<std::wstring> files_;
        Point position_;
    };
    struct EventListener {
        void (*start_)(EventListener*);
        std::type_index type_;
    };

    template<std::execution::receiver R>
    struct Op:EventListener {
       

        friend void tag_invoke(std::execution::start_t, const Op* self) {
            self->start_();
        }
        Op(std::type_index type, EventLoop* loop, R&& r)
            :type_(type), loop_(loop), r_(std::forward<R>(r)) {
            start_ = start__;
        };



        static void start__(Starter* s) {
            auto self = static_cast<Op<R>*>(s);
            try {
                if (std::execution::get_stop_token(std::execution::get_env(r_))..stop_requested()) {
                    std::execution::set_stopped(r_);
                    return;
                }
                std::execution::set_value(r_);
            }
            catch (...) {
                std::execution::set_error(std::current_exception());
            }
        }

        void addListener();
    private:
        EventLoop* loop_;
        R r_;
    };

    class EventListenSender {
        class EventLoop;
    public:
        using completion_signatures = stdexec::completion_signatures<
            stdexec::set_value_t(),
            stdexec::set_error_t(std::exception_ptr),
            stdexec::set_stopped_t()>;
        template<class E>
            requires std::derived_from<E, Event>
        EventListenSender(EventLoop* loop)
            :type_(typeid(E)),loop_(loop){}

        std::execution::empty_env get_env() { return {}; }
        friend std::execution::empty_env tag_invoke(std::execution::get_env_t, const EventListenSender&) {
            return {};
        }

        template<std::execution::receiver R>
        friend Op<R> tag_invoke(std::execution::connect_t, EventListenSender& self,R&& r) {
            return {
                self.type_,
                self.loop_,
                std::forward<R>(r)
            };
        }

    private:
        std::type_index type_;
        EventLoop* loop_;
    };
  

	class EventLoop {
		
	public:
        DLL_API void registerWindow(Window*)const;
        DLL_API bool  pollEvent()const;
        DLL_API void setEvent(const Event*);
        template<class E>
        requires std::derived_from<E,Event>
        auto on() {
            
        }
    protected:
        template<std::execution::receiver R>
        friend struct Op;
        DLL_API void addListener(EventListener* listener);
    private:
        std::shared_mutex mutex_;
        std::unordered_map<std::type_index, std::vector<EventListener*>> listeners_;
	};
    template<std::execution::receiver R>
    inline void Op<R>::addListener()
    {
        loop_->addListener(this);
    }

} // namespace vkd::window