#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <Menu/Menu.h>
#include <fstream>

namespace BitAuth {
	class AuthApi {
	public:
		void Initialize();
		void Authenticate(std::string key);
		void Load_Product();
		void LoadSettings();
		void SaveSettings();
		std::string LoadLogoAuto(std::string url);

		bool HasSpoofer = false;
		bool wantSpoofer = true;
		bool spoofDisk = true;
		bool spoofDiskType = true;
		bool spoofMobo = true;
		bool spoofBoot = true;
		bool spoofMAC = true;
		bool spoofMonitor = true;
		bool spoofGPU = true;
		bool spoofTPM = true;
		bool spoofFileRegistry = true;
		bool spoofStaticSerial = false;
		bool spoofStaticSerialRandomSeed = false;

		class AuthException : public std::runtime_error {
		public:
			explicit AuthException(const std::string& message)
				: std::runtime_error(message) {}
		};

		class data {
		public:
			std::string store_name;
			std::string cheat_name;
			std::string time_left;
			std::string store_logo;
			std::string store_discord_link;
			std::string store_youtube_link;
			std::string store_website_link;
			std::string hexColor;
			int store_menu_color[3];
			bool login_success{};
			bool product_loaded_success{};

			std::string error_message;
		};
		data data;
	private:

	};
}
