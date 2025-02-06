#pragma once
#include <windows.h>
#include <Singleton.h>

namespace LoaderThreads {
	extern void Start();
	extern void ErrorClose(const char*);
	extern void SuccessClose(const char*);
}
