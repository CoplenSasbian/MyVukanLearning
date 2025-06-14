#pragma once

#if defined(_WIN32) 

#include <system_error>
#include <Windows.h>
inline void ThrowIfFailed(bool condition,const std::string& msg){
    if(!condition)
        throw std::system_error(std::error_code(GetLastError(), std::system_category()),msg);
}

#endif