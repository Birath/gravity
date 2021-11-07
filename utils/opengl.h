#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#ifdef _WIN32
#include <Windows.h>
#undef near
#undef far
#undef min
#undef max
#endif
#include <GL/glew.h>
#endif