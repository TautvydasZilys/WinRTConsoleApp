#include "PrecompiledHeader.h"
#include "ConsoleApplication.h"

MIDL_INTERFACE("45D64A29-A63E-4CB6-B498-5781D298CB4F")
ICoreWindowInterop : public IUnknown
{
public:
	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_WindowHandle(
		/* [retval][out] */ __RPC__deref_out_opt HWND *hwnd) = 0;

	virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_MessageHandled(
		/* [in] */ boolean value) = 0;
};

extern "C"
{
	__declspec(dllimport) BOOL WINAPI PostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	__declspec(dllimport) HWND WINAPI GetParent(HWND hWnd);

	__declspec(dllimport) LPWSTR* STDAPICALLTYPE CommandLineToArgvW(LPCWSTR lpCmdLine, int* pNumArgs);
	__declspec(dllimport) HLOCAL WINAPI LocalFree(HLOCAL hMem);	
}

HRESULT WindowToHWND(ABI::Windows::UI::Core::ICoreWindow* coreWindow, HWND* hWnd)
{
	WRL::ComPtr<ICoreWindowInterop> coreWindowInterop;
	HRESULT hr;

	hr = coreWindow->QueryInterface(__uuidof(ICoreWindowInterop), &coreWindowInterop);
	ReturnIfFailed(hr);

	return coreWindowInterop->get_WindowHandle(hWnd);
}

void SendMinimizeMessage(HWND hWnd)
{
	auto parent = GetParent(hWnd);
	PostMessageW(parent, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void SetupCommandLineArgs(wchar_t* commandLine, int* argc, wchar_t*** argv)
{
	*argv = CommandLineToArgvW(commandLine, argc);
}

void FreeCommandLineArgs(wchar_t** argv)
{
	auto result = LocalFree(argv);
	Assert(result == nullptr);
}