#pragma once

#include "Console.h"

HRESULT WindowToHWND(ABI::Windows::UI::Core::ICoreWindow* coreWindow, HWND* hWnd);
void SendMinimizeMessage(HWND hWnd);

void SetupCommandLineArgs(wchar_t* commandLine, int* argc, wchar_t*** argv);
void FreeCommandLineArgs(wchar_t** argv);

template <typename MainFunc>
class ConsoleApplicationView :
	public WRL::RuntimeClass<
	WRL::RuntimeClassFlags<WRL::WinRtClassicComMix>,
	WRL::FtmBase,
	ABI::Windows::ApplicationModel::Core::IFrameworkView,
	ABI::Windows::UI::Core::IDispatchedHandler>
{
private:
	MainFunc m_Main;
	WRL::ComPtr<ABI::Windows::UI::Core::ICoreWindow> m_Window;
	int m_ArgC;
	wchar_t** m_ArgV;

public:
	ConsoleApplicationView(MainFunc mainFunc, int argc, wchar_t** argv) :
		m_Main(std::move(mainFunc)),
		m_ArgC(argc),
		m_ArgV(argv)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE Initialize(ABI::Windows::ApplicationModel::Core::ICoreApplicationView* applicationView) override
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE SetWindow(ABI::Windows::UI::Core::ICoreWindow* window) override
	{
		m_Window = window;
		return window->Activate();
	}

	virtual HRESULT STDMETHODCALLTYPE Load(HSTRING entryPoint) override
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Run() override
	{
		if (m_Window == nullptr)
			return E_FAIL;

		HWND hWnd;
		Console console;
		WRL::ComPtr<ABI::Windows::UI::Core::ICoreDispatcher> dispatcher;

		auto hr = m_Window->get_Dispatcher(&dispatcher);
		ReturnIfFailed(hr);

		hr = WindowToHWND(m_Window.Get(), &hWnd);
		ReturnIfFailed(hr);

		int mainReturnValue;
		std::thread mainThread([this, &mainReturnValue, dispatcher]()
		{
			mainReturnValue = m_Main(m_ArgC, m_ArgV);

			WRL::ComPtr<ABI::Windows::Foundation::IAsyncAction> exitAction;
			auto hr = dispatcher->RunAsync(ABI::Windows::UI::Core::CoreDispatcherPriority_Normal, this, &exitAction);
			Assert(SUCCEEDED(hr));
		});

		int minimizeRetryCount = 5;
		bool isMinimized = false;

		// It seems to fail while we're still on splashscreen, retry 5 times before giving up
		while (!isMinimized && minimizeRetryCount-- > 0)
		{
			SendMinimizeMessage(hWnd);

			hr = dispatcher->ProcessEvents(ABI::Windows::UI::Core::CoreProcessEventsOption_ProcessAllIfPresent);
			Assert(SUCCEEDED(hr));
		}

		hr = dispatcher->ProcessEvents(ABI::Windows::UI::Core::CoreProcessEventsOption_ProcessUntilQuit);
		Assert(SUCCEEDED(hr));

		mainThread.join();

		if (mainReturnValue != 0)
			return E_FAIL;

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Uninitialize()
	{
		m_Window = nullptr;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Invoke() override
	{
		WRL::ComPtr<ABI::Windows::ApplicationModel::Core::ICoreApplicationExit> coreApplicationExit;
		auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.ApplicationModel.Core.CoreApplication").Get(), &coreApplicationExit);
		ReturnIfFailed(hr);

		return coreApplicationExit->Exit();
	}
};

template <typename MainFunc>
class ConsoleApplicationViewSource :
	public WRL::RuntimeClass<
		WRL::RuntimeClassFlags<WRL::WinRtClassicComMix>,
		WRL::FtmBase,
		ABI::Windows::ApplicationModel::Core::IFrameworkViewSource>
{
private:
	MainFunc m_Main;
	int m_ArgC;
	wchar_t** m_ArgV;

public:
	ConsoleApplicationViewSource(MainFunc&& mainFunc, wchar_t* cmdLine) :
		m_Main(std::move(mainFunc))
	{
		SetupCommandLineArgs(cmdLine, &m_ArgC, &m_ArgV);
	}

	~ConsoleApplicationViewSource()
	{
		FreeCommandLineArgs(m_ArgV);
	}

	virtual HRESULT STDMETHODCALLTYPE CreateView(ABI::Windows::ApplicationModel::Core::IFrameworkView** viewProvider) override
	{
		*viewProvider = WRL::Make<ConsoleApplicationView<MainFunc>>(m_Main, m_ArgC, m_ArgV).Detach();
		return S_OK;
	}
};

template <typename MainFunc>
static HRESULT RunConsoleApplication(MainFunc mainFunc, wchar_t* cmdLine)
{
	auto viewSource = WRL::Make<ConsoleApplicationViewSource<MainFunc>>(std::forward<MainFunc>(mainFunc), cmdLine);

	WRL::ComPtr<ABI::Windows::ApplicationModel::Core::ICoreApplication> coreApplicationStatics;
	auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.ApplicationModel.Core.CoreApplication").Get(), &coreApplicationStatics);
	ReturnIfFailed(hr);

	return coreApplicationStatics->Run(viewSource.Get());
}