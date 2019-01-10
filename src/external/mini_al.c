// The implementation of mini_al needs to #include windows.h which means it needs to go into
// it's own translation unit. Not doing this will cause conflicts with CloseWindow(), etc.
#define MINI_AL_IMPLEMENTATION
#define MAL_NO_JACK
#define MAL_NO_OPENAL
#define MAL_NO_SDL
//#define MAL_NO_NULL
#include "mini_al.h"
