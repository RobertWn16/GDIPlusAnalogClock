#include "GDIWindow.hpp"

GDIWindow::GDIWindow()
{
	this->corehWnd = nullptr;
}

GDIWindow::~GDIWindow()
{
    DeleteObject(wndClass.hbrBackground);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    DestroyWindow(this->corehWnd);
    UnregisterClass(L"GDIClass", wndClass.hInstance);
}

HRESULT _stdcall GDIWindow::CreateGDIWindow(HINSTANCE hInstance, WNDPROC WndProc, LONG_PTR lParam)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    this->wndClass.style = CS_HREDRAW | CS_VREDRAW;
    this->wndClass.lpfnWndProc = WndProc;
    this->wndClass.cbClsExtra = 0;
    this->wndClass.cbWndExtra = 0;
    this->wndClass.hInstance = hInstance;
    this->wndClass.hIcon = 0;
    this->wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    this->wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    this->wndClass.lpszMenuName = NULL;
    this->wndClass.lpszClassName = TEXT("GDIClass");
    this->wndClass.lpszMenuName = TEXT("SysTrayMenu");

    if (!RegisterClass(&wndClass))
        return HRESULT_FROM_WIN32(ERROR_CLASS_DOES_NOT_EXIST);

    this->corehWnd = CreateWindowEx(
        WS_EX_LAYERED,
        TEXT("GDIClass"),   // window class name
        TEXT("AnalogClock"),  // window caption
        WS_DLGFRAME,      // window style
        0,            // initial x position
        0,            // initial y position
        500,            // initial x size
        500,            // initial y size
        NULL,                     // parent window handle
        NULL,                     // window menu handle
        hInstance,                // program instance handle
        (LPVOID)lParam);

}

void _stdcall GDIWindow::Start()
{
    ShowWindow(this->corehWnd, SW_SHOWDEFAULT);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
}