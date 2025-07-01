
#ifndef DLL_API_H
#define DLL_API_H

#ifdef VKD_STATIC_DEFINE
#  define DLL_API
#  define VKD_NO_EXPORT
#else
#  ifndef DLL_API
#    ifdef vkd_EXPORTS
        /* We are building this library */
#      define DLL_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define DLL_API __declspec(dllimport)
#    endif
#  endif

#  ifndef VKD_NO_EXPORT
#    define VKD_NO_EXPORT 
#  endif
#endif

#ifndef VKD_DEPRECATED
#  define VKD_DEPRECATED __declspec(deprecated)
#endif

#ifndef VKD_DEPRECATED_EXPORT
#  define VKD_DEPRECATED_EXPORT DLL_API VKD_DEPRECATED
#endif

#ifndef VKD_DEPRECATED_NO_EXPORT
#  define VKD_DEPRECATED_NO_EXPORT VKD_NO_EXPORT VKD_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VKD_NO_DEPRECATED
#    define VKD_NO_DEPRECATED
#  endif
#endif

#endif /* DLL_API_H */
