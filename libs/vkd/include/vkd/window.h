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
        Rect(const T& rect) {
            x = std::get<0>(rect);
            y = std::get<1>(rect);
            width = std::get<2>(rect);
            height = std::get<3>(rect);
        }
        bool operator==(const Rect& other) const {
            return x == other.x && y == other.y && width == other.width && height == other.height;
        }
    };



    typedef struct Vec2 {
        union
        {
            int x;
            int width;
        };
        union
        {
            int y;
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

    } Size, Point;



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
            const Rect& rect = { -1,-1,800,600 });
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
        bool hasPreventDefault() const {
            return preventDefault_;
        }
        std::type_index type() const {
            return typeid (*this);
        }

        virtual ~Event() {}
        DLL_API Event(const NativeEvent&);
    protected:
        friend class EventLoop;
        Window* window_;
        bool preventDefault_ = false;
    };
    class CloseEvent :public Event {
        friend class EventLoop;
    public:
        using Event::Event;
    };
    class QuitEvent :public Event
    {
        friend class EventLoop;
    public:
        using Event::Event;
    };



	class SizeEvent :public Event {
		friend class EventLoop;
	public:
		const Size& size()const {
			return size_;
		};
        DLL_API SizeEvent(const NativeEvent&);


    protected:
		Size size_;
	};

    

	class MoveEvent : public Event {
		friend class EventLoop;
	public:
		const Point& position() const {
			return position_;
		}
        DLL_API MoveEvent(const NativeEvent&);

    protected:
		Point position_; 
	};

	class FocusEvent : public Event {
		friend class EventLoop;
	public:
		bool gained() const {
			return gained_;
		}
        DLL_API FocusEvent(const NativeEvent&);

    protected:
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
        DLL_API KeyEvent(const NativeEvent&);

    protected:

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
        DLL_API MouseEvent(const NativeEvent&);

    protected:
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
        DLL_API WheelEvent(const NativeEvent&);

    protected:
        int delta_;     
        Point position_;
    };

    class PaintEvent : public Event {
        friend class EventLoop;
    public:
        const Rect& region() const {
            return region_;
        }  
        DLL_API PaintEvent(const NativeEvent&);

    protected:
        Rect region_;  
    };

    class ShowEvent : public Event {
        friend class EventLoop;
    public:
        bool shown() const {
            return shown_;
        }  
        DLL_API ShowEvent(const NativeEvent&);

    protected:
        bool shown_:1;
    };

    class IconifyEvent : public Event {
        friend class EventLoop;
    public:
        bool iconified() const {
            return iconified_;
        }  
        DLL_API IconifyEvent(const NativeEvent&);

    protected:
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
        DLL_API FileDropEvent(const NativeEvent&);

    protected:
        std::vector<std::wstring> files_;
        Point position_;
    };



    class EventLoop;


    struct EventListener {
        void (*start_)(EventListener*,const Event* e) noexcept;
        std::type_index type_;

        EventListener(decltype(start_) start, std::type_index type)
            :start_(start), type_(type)
        {};
        EventListener(EventListener&&) noexcept = default;
        EventListener(const EventListener&)noexcept = default;
    };

    template<std::execution::receiver R,class E>
    struct Op:public EventListener {
       
        Op(EventLoop* loop, R&& r)
            :EventListener(start__, typeid(E)),  loop_(loop), r_(std::forward<R>(r)) {
          
        }
        Op(Op&&) noexcept = default;
        
        Op(const Op&)noexcept = default;

        static void start__(EventListener* s,const Event* e) noexcept {
            auto self = static_cast<Op*>(s);
            try {
                if (std::execution::get_stop_token(std::execution::get_env(self->r_)).stop_requested()) {
                    std::execution::set_stopped(self->r_);
                    return;
                }
                std::execution::set_value(self->r_,(E*)(e));
            }
            catch (...) {
                std::execution::set_error(self->r_,std::current_exception());
            }
        }

         void addListener() noexcept;

        ~Op(){}

        friend void tag_invoke(std::execution::start_t, Op & self)noexcept{
            self.start();
        }
        void start() noexcept{
            addListener();
        }

        //static_assert(std::execution::operation_state<Op>);
    private:
        EventLoop* loop_;
        R r_;
    };


    template<class E>
    class EventListenSender:public std::execution::sender_t {
    public:
        using completion_signatures = std::execution::completion_signatures<
            std::execution::set_value_t(const E*),
            std::execution::set_error_t(std::exception_ptr),
            std::execution::set_stopped_t()>;
        
        EventListenSender(EventLoop* loop)
            :loop_(loop){
        }

		EventListenSender(EventListenSender&&)noexcept = default;
		EventListenSender(const EventListenSender&) = default;


        template<std::execution::receiver R>
        friend Op<R,E> tag_invoke(std::execution::connect_t,EventListenSender& self,R&& r) noexcept{
            return {
                self.loop_,
                std::forward<R>(r)
            };
        }

    private:
        EventLoop* loop_;

    };
  

	class DLL_API EventLoop {
		
	public:
         EventLoop();
         void registerWindow(Window*)const;
         bool  pollEvent();
         void setEvent(const Event*);

         void postQuitEvent() const;

        template<class E>
        requires std::derived_from<E,Event>
        EventListenSender<E> on() {
            return EventListenSender<E>(this);
        }
    protected:
        template<std::execution::receiver R,class E>
        friend struct Op;
        void addListener(EventListener* listener);
    private:
        std::shared_mutex mutex_;
        std::unordered_map<std::type_index, std::vector<EventListener*>> listeners_;
	};

    template<std::execution::receiver R,class E>
    inline void Op<R,E>::addListener()  noexcept
    {
        loop_->addListener(this);
    }

  

} // namespace vkd::window