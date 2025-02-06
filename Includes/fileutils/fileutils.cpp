#define WIN32_LEAN_AND_MEAN
#include "fileutils.h"
#include <io.h>
#define access _access_s
#include <stdio.h>
#include <fstream>
#include <Psapi.h>
#include <TlHelp32.h>
#include <comdef.h>
#include <shellapi.h>
#include <ctime>
#include <direct.h>
#include <random>
#include <direct.h>
#include <iostream>
#include <istream>
#include <sstream>

#include <codecvt>


std::string fileutils::RandomString(std::size_t length)
{
	const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

	std::string random_string;

	for (std::size_t i = 0; i < length; ++i)
	{
		random_string += CHARACTERS[distribution(generator)];
	}

	return random_string;
}

VOID fileutils::Terminate()
{
	Sleep(4000);
	ExitProcess(0);
}
