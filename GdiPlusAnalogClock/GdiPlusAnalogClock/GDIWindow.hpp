#include <Windows.h>
#include <windowsx.h>
#include <objidl.h>
#include <gdiplus.h>

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib, "winmm.lib")

class GDIWindow
{
protected:
	HWND corehWnd;
	WNDCLASS wndClass;
	ULONG_PTR gdiplusToken;
public:
	GDIWindow();
	HRESULT _stdcall CreateGDIWindow(HINSTANCE, WNDPROC, LONG_PTR);
	void _stdcall Start();
	
	~GDIWindow();
};