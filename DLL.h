#ifndef _DLL_H
#define _DLL_H

#ifdef __cplusplus
extern "C" {
#endif

#define DLL_EXPORT

DLL_EXPORT int add(int a, int b);

#ifdef __cplusplus
}
#endif

#endif // _DLL_H
