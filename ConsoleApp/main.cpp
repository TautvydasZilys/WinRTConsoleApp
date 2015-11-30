#include "PrecompiledHeader.h"
#include "ConsoleApplication.h"

int wmain(int argc, wchar_t** argv);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	return RunConsoleApplication([](int argc, wchar_t** argv)
	{
		return wmain(argc, argv);
	}, lpCmdLine);
}

int wmain(int argc, wchar_t** argv)
{
	std::string name;

	std::cout << "What's your name?" << std::endl;
	std::cin >> name;
	std::cout << "Hello, " << name << "!" << std::endl;

	return 0;
}