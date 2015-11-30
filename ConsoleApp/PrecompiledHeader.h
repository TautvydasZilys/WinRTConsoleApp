#pragma once

#include <Windows.h>
#include <Windows.ApplicationModel.h>
#include <Windows.ApplicationModel.Core.h>
#include <wrl.h>

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace WRL
{
	using namespace Microsoft::WRL;
	using namespace Microsoft::WRL::Wrappers;
}

#if _DEBUG
#define Assert(x) do { if (!(x)) __debugbreak(); } while (false)
#else
#define Assert(x) do { } while (false)
#endif

#define ReturnIfFailed(hr) do { if (FAILED(hr)) { Assert(false); return hr; } } while (false)