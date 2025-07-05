#pragma once

#include "../core.h"
#include "../execution/stdexec.h"
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


   template<class E>
   concept EventType = std::derived_from<E, Event>;


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


 
 
    struct ListenerHolder {
        ListenerHolder(std::type_index type):type_(type){};
        std::type_index type_;
        void(*exec_)(ListenerHolder*, const Event*)noexcept
            = nullptr;
    };
    template<class Task>
    class Listener:public ListenerHolder {
    public:

        Listener(Task&& task,std::type_index type)
        :ListenerHolder(type),task_(std::forward<Task>(task)){
            exec_ = __exec_;
        }
      
    private:
        static void __exec_(ListenerHolder* h, const Event* e) noexcept{
            auto self = (Listener*)h;
            try{
                std::execution::start_detached(
                    std::execution::just(e)
                    | self->task_
                );
            }
            catch (...)
            {
               
            }
        }

        Task task_;
    };

    class DLL_API EventLoop {

    public:
        EventLoop();
        void registerWindow(Window*)const;
        bool  pollEvent() noexcept;
        void setEvent(const Event*);

        void postQuitEvent() const;

        template<EventType E, class S>
        std::unique_ptr<ListenerHolder> on(S&& s) {
            auto task = std::execution::then([](const Event* e) {return (E*)e;}) 
                | std::forward<S>( s);

            auto listener = std::make_unique<Listener<decltype(task)>>(std::move(task), std::type_index{typeid(E)});
            ___addListenerHolder(listener.get());
            return listener;
        }
        template<class T>
            requires std::derived_from<T, ListenerHolder> || std::same_as<T, ListenerHolder>
        void pauseListener(const std::unique_ptr<T>& listener){
            ___removeListenerHolder(listener.get());
        }
        template<class T>
            requires std::derived_from<T, ListenerHolder> || std::same_as<T, ListenerHolder>
        void resumeListener(const std::unique_ptr<T>& listener) {
            ___addListenerHolder(listener.get());
        }


    protected:

        void ___addListenerHolder(ListenerHolder*);
        
        void ___removeListenerHolder(ListenerHolder*);
       
    private:
        std::shared_mutex mutex_;
        std::unordered_map<std::type_index, std::vector<ListenerHolder*>> listeners_;
	};

    using EventListenHolder = std::unique_ptr<ListenerHolder>;
 


  

} // namespace vkd::window