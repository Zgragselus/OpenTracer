#ifdef OPENTRACER_EXPORTS
#define OPENTRACER_API __declspec(dllexport)
#else
#define OPENTRACER_API __declspec(dllimport)
#endif