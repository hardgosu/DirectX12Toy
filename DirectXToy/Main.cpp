#include "CompleteHeader.h"
#include "DirectXToy.h"

CREATE_APPLICATION(Toy::DirectXToy)

HWND g_hWnd = nullptr;
uint32_t g_DisplayWidth = 1440;
uint32_t g_DisplayHeight = 960;
ProcedureMap g_ProcedureMap;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto found = g_ProcedureMap.equal_range(message);
	for (auto begin = found.first; begin != found.second; ++begin)
	{
		begin->second(hWnd, message, wParam, lParam);
	}

	if (std::distance(found.first, found.second) == 0)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


int RunApplication(IGameApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow)
{
	Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
	ASSERT_SUCCEEDED(InitializeWinRT);

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = className;
	wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
	ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");

	// Create window
	RECT rc = { 0, 0, (LONG)g_DisplayWidth, (LONG)g_DisplayHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

	ASSERT(g_hWnd != 0);

	//InitializeApplication(app);
	app.Startup();

	ShowWindow(g_hWnd, nCmdShow/*SW_SHOWDEFAULT*/);

	UINT64 countsPerSec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
	const double secondsPerFrequency = 1.0 / countsPerSec;
	double frameTime{};

	UINT64 begin{};
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&begin));
	do
	{
		MSG msg = {};
		bool done = false;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;
		if (GetForegroundWindow() != g_hWnd)
		{
			continue;
		}
		//Tick
		UINT64 end{};
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&end));
		frameTime = (end - begin) * secondsPerFrequency;
		begin = end;

		app.Update(frameTime);
	} while (!app.IsDone());	// Returns false to quit loop

	//TerminateApplication(app);
	//Graphics::Shutdown();
	app.Cleanup();
	return 0;
}
