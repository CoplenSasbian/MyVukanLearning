

#ifdef WIN32
#include<vkd/exception/system_error.h>
#include<Windows.h>
#include<memory>
#include <format>
namespace vkd {

    struct LocalFreeDeleter {
        void operator()(void* ptr) const noexcept {
            if (ptr) ::LocalFree(ptr);
        }
    };
    using LocalStringPtr = std::unique_ptr<char, LocalFreeDeleter>;

    std::string FormatSystemErrorMessage(
        std::string_view context,
        HWND hWnd,
        DWORD errorCode 
    ) {
        // 尝试获取系统错误消息
        char* rawMsg = nullptr;
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;

        DWORD size = ::FormatMessageA(
            flags,
            nullptr,
            errorCode,
            MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&rawMsg),
            0,
            nullptr
        );

        LocalStringPtr systemMsg(rawMsg);

        std::string_view cleanMsg = rawMsg ? rawMsg : "Unknown system error";
        while (!cleanMsg.empty() &&
            (cleanMsg.back() == '\n' || cleanMsg.back() == '\r')) {
            cleanMsg.remove_suffix(1);
        }

        if (hWnd) {
            return std::format(
                "{} (Native handle: 0x{:016X})\n"
                "[Error {} (0x{:08X})] {}",
                context,
                reinterpret_cast<uintptr_t>(hWnd),
                errorCode,
                errorCode,
                cleanMsg
            );
        }

        return std::format(
            "{}\n[Error {} (0x{:08X})] {}",
            context,
            errorCode,
            errorCode,
            cleanMsg
        );
    }

	SystemError::SystemError(const std::string& context, void* handle, uint32_t errcode)
		:std::runtime_error(FormatSystemErrorMessage(context, (HWND)handle,errcode)) ,errorCode_(errcode)
	{}

	std::uint32_t  SystemError::getLastError() noexcept {
		return ::GetLastError();
	}
}


#endif // WIN32

