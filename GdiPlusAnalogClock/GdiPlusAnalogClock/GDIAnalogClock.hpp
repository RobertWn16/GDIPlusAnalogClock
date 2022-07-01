#include "GDIWindow.hpp"

#define IDT_ABSOLUTETIMER 0
#define IDT_ANGLEFACTOR 6

#define IDI_SYSTRAY_CLOCK 100
#define WM_USER_CLOCK_SHELL (WM_USER + 1)
#define IDM_ENABLE_CLOCK (WM_USER + 2)
#define IDM_DISABLE_CLOCK (WM_USER + 3)
#define IDM_CLOSE_CLOCK (WM_USER + 4)

LRESULT CALLBACK GDIWindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class GDIAnalogClock : GDIWindow
{
private:
	Gdiplus::Bitmap* bmClockPanel;
	Gdiplus::Bitmap* bmHourHand;
	Gdiplus::Bitmap* bmMinuteHand;
	Gdiplus::Bitmap* bmSecondHand;
	Gdiplus::Graphics* gdiGraphics;

	HDC coreHdc;
	HDC gdiDrawingDC;
	HBITMAP currentBitmap;
	HBITMAP oldBitmap;
	HICON sysIcon;
	NOTIFYICONDATA ntfData;

	FLOAT zoomLevel;
	INT currentWidth;
	INT currentHeigth;
	INT currentAngle;
	INT currentAlpha;
	SYSTEMTIME sysTime;
	BOOL isWindowVisible;

public:
	GDIAnalogClock();

	void _stdcall SetWindowPosition();
	HRESULT _stdcall Init_Load(LPCWSTR clockPanelPath, LPCWSTR hourHandPath, LPCWSTR minuteHandPath, LPCWSTR secondHandPath, HINSTANCE hInstance, WNDPROC WndProc = GDIWindowProcedure);
	HRESULT _stdcall LoadClockPanel(LPCWSTR);
	HRESULT _stdcall LoadHourHand(LPCWSTR);
	HRESULT _stdcall LoadMinuteHand(LPCWSTR);
	HRESULT _stdcall LoadSecondHand(LPCWSTR);

	HRESULT _stdcall DrawClockPanel();
	HRESULT _stdcall DrawHourHand(INT);
	HRESULT _stdcall DrawMinuteHand(INT);
	HRESULT _stdcall DrawSecondHand(INT);


	void _stdcall Begin(HINSTANCE, WNDPROC WndProc = GDIWindowProcedure);
	void _stdcall BeginDraw();
	void _stdcall EndDraw();
	void _stdcall Update(BYTE alpha);
	void _stdcall DrawClock(INT);

	inline void _stdcall SetAlpha(INT);
	inline void _stdcall SetWindowState(BOOL);

	inline HWND _stdcall GetWindID();
	inline INT _stdcall GetAlpha();
	inline BOOL _stdcall GetWindowState();

	~GDIAnalogClock();
};