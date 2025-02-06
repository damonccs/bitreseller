#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_INTERNAL_ERROR ((NTSTATUS)0xC00000E5)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

namespace fileutils
{
	std::string RandomString(std::size_t length);
	VOID Terminate();
}
