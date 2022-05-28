/*
Required file to make miniaudio.h compile to Switch
*/

#define RTLD_LAZY         0x00001
#define RTLD_NOW          0x00002
#define RTLD_BINDING_MASK 0x3
#define RTLD_NOLOAD       0x00004
#define RTLD_DEEPBIND     0x00008
#define RTLD_GLOBAL       0x00100
#define RTLD_LOCAL        0
#define RTLD_NODELETE     0x01000

void *dlopen(const char *, int);
void *dlsym(void *, const char *);
int  dlclose(void *);
char *dlerror(void);

int pthread_attr_setschedpolicy(pthread_attr_t *, int) { return 0; }
int sched_get_priority_max(int policy) { return 0; }
int sched_get_priority_min(int policy) { return 0; }
