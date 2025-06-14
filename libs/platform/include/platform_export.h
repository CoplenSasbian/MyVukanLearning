
#ifndef PLT_EXPORT_H
#define PLT_EXPORT_H

#ifdef PLATFORM_STATIC_DEFINE
#  define PLT_EXPORT
#  define PLATFORM_NO_EXPORT
#else
#  ifndef PLT_EXPORT
#    ifdef platform_EXPORTS
        /* We are building this library */
#      define PLT_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define PLT_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef PLATFORM_NO_EXPORT
#    define PLATFORM_NO_EXPORT 
#  endif
#endif

#ifndef PLATFORM_DEPRECATED
#  define PLATFORM_DEPRECATED __declspec(deprecated)
#endif

#ifndef PLATFORM_DEPRECATED_EXPORT
#  define PLATFORM_DEPRECATED_EXPORT PLT_EXPORT PLATFORM_DEPRECATED
#endif

#ifndef PLATFORM_DEPRECATED_NO_EXPORT
#  define PLATFORM_DEPRECATED_NO_EXPORT PLATFORM_NO_EXPORT PLATFORM_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef PLATFORM_NO_DEPRECATED
#    define PLATFORM_NO_DEPRECATED
#  endif
#endif

#endif /* PLT_EXPORT_H */
