#pragma once
#include <windows.h>
#include <Singleton.h>
#include <string>
#include <vector>


class Menu
	: public Singleton<Menu>
{
	friend class Singleton<Menu>;
	Menu(void)
	{
	}

	Menu(Menu const&) = default;

	Menu(Menu&&) = default;

	Menu& operator=(Menu const&) = default;

	Menu& operator=(Menu&&) = default;

	~Menu(void)
	{
	}
public:

	bool Start();
	void Loop();
	HINSTANCE instance = NULL;
	bool Loading = false;
	bool isError = true;
	bool hasSpoofer = false;

	std::string message = "";
	std::string key = "";

	std::string store_name;
	std::string cheat_name;
	std::string time_left;
	std::string store_logo;
	std::string store_discord_link;
	std::string store_youtube_link;
	std::string store_website_link;
	int store_menu_color;
	int page = 0;
};