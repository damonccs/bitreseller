
#include <windows.h>
#include <fstream>
#include <TlHelp32.h>
#include "LoaderThreads/LoaderThreads.h"
#include "fileutils/fileutils.h"
#include "Menu/Menu.h"
#include <Server/Server.h>
#include <Menu/imgui/imgui.h>
#include <main.h>

using namespace std::chrono_literals;

//Memory Enumeration
#pragma comment(lib, "Iphlpapi.lib")

//cpprest
#pragma comment(lib, "brotlicommon-static.lib")
#pragma comment(lib, "brotlidec-static.lib")
#pragma comment(lib, "brotlienc-static.lib")
#pragma comment(lib, "cpprest_2_10.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

//tpm check
#pragma comment(lib, "Tbs.lib")

//timeGetTime
#pragma comment(lib, "Winmm.lib")

//float accent_color[4] = { 0/* / 255.f, 145 / 255.f, 255 / 255.f, 1.f */};
float accent_color[4];

void ConvertHexToRGBA(const std::string& hexColor) {
    if (hexColor.size() == 9 && hexColor[0] == '#') {
        int r, g, b, a;

        std::stringstream ss;
        ss << std::hex << hexColor.substr(1, 2);
        ss >> r;
        ss.clear();
        ss.str("");

        ss << std::hex << hexColor.substr(3, 2);
        ss >> g;
        ss.clear();
        ss.str("");

        ss << std::hex << hexColor.substr(5, 2);
        ss >> b;
        ss.clear();
        ss.str("");

        ss << std::hex << hexColor.substr(7, 2);
        ss >> a;

        // Assign to accent_color as floats normalized between 0 and 1
        accent_color[0] = r / 255.0f;
        accent_color[1] = g / 255.0f;
        accent_color[2] = b / 255.0f;
        accent_color[3] = a / 255.0f;
    }
   /* else {
        MessageBoxA(NULL, "Invalid hex color format", "Error", MB_OK | MB_ICONERROR);
    }*/
}
int CALLBACK WinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       lpCmdLine,
	int         nCmdShow
)
{
	BitAuth::AuthApi BitAuthApp;

	Menu::Get().instance = hInstance;

	BitAuthApp.Initialize();

    ConvertHexToRGBA(BitAuthApp.data.hexColor);


	Menu::Get().store_logo = BitAuthApp.data.store_logo;
    Menu::Get().store_name = BitAuthApp.data.store_name;
    Menu::Get().store_website_link = BitAuthApp.data.store_website_link;
    Menu::Get().store_youtube_link = BitAuthApp.data.store_youtube_link;
    Menu::Get().store_discord_link = BitAuthApp.data.store_discord_link;


	//Start Threads
	LoaderThreads::Start();

	return 0;
}