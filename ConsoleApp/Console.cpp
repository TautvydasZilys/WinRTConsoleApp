#include "PrecompiledHeader.h"
#include "Console.h"

extern "C"
{
	__declspec(dllimport) BOOL WINAPI AllocConsole();
	__declspec(dllimport) BOOL WINAPI FreeConsole();
	__declspec(dllimport) HANDLE WINAPI GetStdHandle(DWORD nStdHandle);
}

struct StdBuffers
{
	static const size_t kBufferSize = 64 * 1024;
	uint8_t stdOutBuffer[kBufferSize];
	uint8_t stdInBuffer[kBufferSize];
	uint8_t stdErrBuffer[kBufferSize];
};

template <size_t bufferSize>
static void RedirectStdToConsole(DWORD stdHandleType, const char* openMode, FILE* fileToOverwrite, uint8_t (&bufferingBuffer)[bufferSize])
{
	auto stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(stdHandleType));
	Assert(stdHandle != 0 && stdHandle != reinterpret_cast<intptr_t>(INVALID_HANDLE_VALUE));

	auto stdFileDescriptor = _open_osfhandle(stdHandle, _O_TEXT);
	Assert(stdFileDescriptor != -1);

	auto stdFile = _fdopen(stdFileDescriptor, openMode);
	Assert(stdFile != nullptr);

	auto setBufferingResult = setvbuf(stdout, reinterpret_cast<char*>(bufferingBuffer), _IOLBF, bufferSize);
	Assert(setBufferingResult == 0);

	*fileToOverwrite = *stdFile;
}

Console::Console()
{
	m_StdBuffers = std::make_unique<StdBuffers>();

	auto result = AllocConsole();
	Assert(result != FALSE);

	RedirectStdToConsole(STD_OUTPUT_HANDLE, "w", stdout, m_StdBuffers->stdOutBuffer);
	RedirectStdToConsole(STD_INPUT_HANDLE, "r", stdin, m_StdBuffers->stdInBuffer);
	RedirectStdToConsole(STD_ERROR_HANDLE, "w", stderr, m_StdBuffers->stdErrBuffer);

	std::ios::sync_with_stdio();
}

Console::~Console()
{
	auto result = FreeConsole();
	Assert(result != FALSE);
}
