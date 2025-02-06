#include "LoaderThreads.h"
#include <mutex>
#include <thread>
#include <iostream>
#include <document.h>
#include <writer.h>
#include "fileutils/fileutils.h"
#include "Menu/Menu.h"
#include "Server/Server.h"

VOID EntryPoint() {
	BitAuth::AuthApi BitAuthApp;

	Menu::Get().key.reserve(255);
	Menu::Get().Start();
	Menu::Get().Loop();
	CloseHandle(GetCurrentThread());
}

void LoaderThreads::Start() {
	EntryPoint();
}
