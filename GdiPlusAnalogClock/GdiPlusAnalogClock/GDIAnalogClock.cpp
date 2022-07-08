#include "GDIAnalogClock.hpp"
#include <math.h>
#include <shellscalingapi.h>
#include <shellapi.h>

#pragma comment(lib, "Shcore.lib")

#define HOVER_TIMEOUT 1
#define MAX_WIDTH_4K 3840
#define MAX_HEIGTH_4K 2160
const float MAX_ZOOM_4K = 0.3f;

LRESULT CALLBACK GDIWindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GDIAnalogClock* _this = (GDIAnalogClock*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
		break;

	case WM_USER_CLOCK_SHELL:
		switch (LOWORD(lParam))
		{
		case WM_RBUTTONDOWN:
			UINT uFlag = MF_BYPOSITION | MF_STRING;
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			HMENU hPopMenu = CreatePopupMenu();
			if(_this->GetWindowState() == FALSE)
				InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_ENABLE_CLOCK, L"&Enable");
			else
				InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_DISABLE_CLOCK, L"&Disable");

			InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_CLOSE_CLOCK, L"&Close");
			SetForegroundWindow(hWnd);
			int _trckCode = TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_RETURNCMD, cursorPos.x, cursorPos.y, 0, hWnd, NULL);
			if (_trckCode == IDM_ENABLE_CLOCK)
			{
				_this->SetWindowState(TRUE);
				ShowWindow(hWnd, SW_SHOWDEFAULT);
			}
			else if (_trckCode == IDM_DISABLE_CLOCK)
			{
				_this->SetWindowState(FALSE);
				ShowWindow(hWnd, SW_HIDE);
			}
			else if (_trckCode == IDM_CLOSE_CLOCK)
			{
				SendMessage(hWnd, WM_CLOSE, wParam, lParam);
			}

			DestroyMenu(hPopMenu);
		
		}
		break;

	case WM_TIMER:
		switch (wParam)
		{
		case IDT_ABSOLUTETIMER:
			_this->DrawClock(_this->GetAlpha());
			break;
		default:
			break;
		}

	case WM_MOUSEMOVE:
		TRACKMOUSEEVENT trckMouseEv;
		trckMouseEv.cbSize = sizeof(TRACKMOUSEEVENT);
		trckMouseEv.dwFlags = TME_HOVER | TME_LEAVE;
		trckMouseEv.dwHoverTime = HOVER_TIMEOUT;
		trckMouseEv.hwndTrack = _this->GetWindID();
		TrackMouseEvent(&trckMouseEv);
		break;

	case WM_MOUSEHOVER:
		_this->SetAlpha(255);
		_this->DrawClock(_this->GetAlpha());
		break;
	case WM_MOUSELEAVE:
		_this->SetAlpha(128);
		_this->DrawClock(_this->GetAlpha());
		break;

	case WM_LBUTTONDOWN:
		PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;

	case WM_MBUTTONDOWN:
		SendMessage(hWnd, WM_CLOSE, wParam, lParam);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		DestroyWindow(hWnd);
		break;

	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

GDIAnalogClock::GDIAnalogClock()
{
	this->bmHourHand = nullptr;
	this->bmMinuteHand = nullptr;
	this->bmSecondHand = nullptr;

	this->coreHdc = nullptr;
	this->gdiDrawingDC = nullptr;
	ZeroMemory(&this->currentBitmap, sizeof(HBITMAP));
	ZeroMemory(&this->oldBitmap, sizeof(HBITMAP));
	ZeroMemory(&this->sysTime, sizeof(SYSTEMTIME));
	ZeroMemory(&this->ntfData, sizeof(NOTIFYICONDATA));

	this->zoomLevel = 0.3f;
	this->currentAngle = 0;
	this->currentAlpha = 128;
	this->isWindowVisible = TRUE;
}

void _stdcall GDIAnalogClock::SetWindowPosition()
{
	this->currentWidth = static_cast<INT>(floor(double(this->bmClockPanel->GetWidth() * 1)));
	this->currentHeigth = static_cast<INT>(floor(double(this->bmClockPanel->GetHeight() * 1)));

	UINT dpX;
	UINT dpY;
	HMONITOR hMonitor;
	MONITORINFO monitorInfo;
	ZeroMemory(&monitorInfo, sizeof(MONITORINFO));
	monitorInfo.cbSize = sizeof(MONITORINFO);

	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	hMonitor = MonitorFromWindow(this->corehWnd, MONITOR_DEFAULTTOPRIMARY);
	GetMonitorInfo(hMonitor, &monitorInfo);

	int hWndWidth = monitorInfo.rcMonitor.right - currentWidth * zoomLevel;

	this->zoomLevel = MAX_ZOOM_4K * (MAX_WIDTH_4K / hWndWidth);
	SetWindowPos(this->corehWnd, NULL, monitorInfo.rcMonitor.right - currentWidth * zoomLevel, 0,  currentWidth, currentHeigth, SWP_SHOWWINDOW);

}

HRESULT _stdcall GDIAnalogClock::Init_Load(LPCWSTR clockPanelPath, LPCWSTR hourHandPath, LPCWSTR minuteHandPath, LPCWSTR secondHandPath, HINSTANCE hInstance, WNDPROC WndProc)
{
	this->CreateGDIWindow(hInstance, WndProc, (LONG_PTR)this);
	this->LoadClockPanel(clockPanelPath);
	this->LoadHourHand(hourHandPath);
	this->LoadMinuteHand(minuteHandPath);
	this->LoadSecondHand(secondHandPath);
	return S_OK;
}

HRESULT _stdcall GDIAnalogClock::LoadClockPanel(LPCWSTR clockPanelPath)
{
	this->bmClockPanel = Gdiplus::Bitmap::FromFile(clockPanelPath);
	if (this->bmClockPanel == nullptr)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	return S_OK;
}

HRESULT _stdcall GDIAnalogClock::LoadHourHand(LPCWSTR hourHandPath)
{
	this->bmHourHand = Gdiplus::Bitmap::FromFile(hourHandPath);
	if (this->bmHourHand == nullptr)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	return S_OK;
}

HRESULT _stdcall GDIAnalogClock::LoadMinuteHand(LPCWSTR minuteHandPath)
{
	this->bmMinuteHand= Gdiplus::Bitmap::FromFile(minuteHandPath);
	if (this->bmMinuteHand == nullptr)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	return S_OK;
}

HRESULT _stdcall GDIAnalogClock::LoadSecondHand(LPCWSTR secondHandPath)
{
	this->bmSecondHand = Gdiplus::Bitmap::FromFile(secondHandPath);
	if (this->bmSecondHand == nullptr)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	return S_OK;
}


HRESULT _stdcall GDIAnalogClock::DrawClockPanel()
{
	this->currentWidth = static_cast<INT>(floor(double(this->bmClockPanel->GetWidth() * 1)));
	this->currentHeigth = static_cast<INT>(floor(double(this->bmClockPanel->GetHeight() * 1)));

	Gdiplus::Rect imageCoord (0, 0, currentWidth, currentHeigth);
	this->gdiGraphics->ScaleTransform(zoomLevel, zoomLevel);
	gdiGraphics->DrawImage(this->bmClockPanel, imageCoord);
	gdiGraphics->ResetTransform();
	return S_OK;
}

HRESULT _stdcall GDIAnalogClock::DrawHourHand(INT angle)
{
	this->currentWidth = static_cast<INT>(floor(double(this->bmClockPanel->GetWidth() * zoomLevel)));
	this->currentHeigth = static_cast<INT>(floor(double(this->bmClockPanel->GetHeight() * zoomLevel)));


	this->gdiGraphics->ScaleTransform(zoomLevel, zoomLevel);
	this->gdiGraphics->TranslateTransform(this->currentWidth / 2, this->currentHeigth / 2, Gdiplus::MatrixOrderAppend);
	this->gdiGraphics->RotateTransform(angle);

	this->gdiGraphics->DrawImage(this->bmHourHand, -25, -25, this->bmHourHand->GetWidth() / 2, this->bmClockPanel->GetHeight() / 2 - 120);
	this->gdiGraphics->ResetTransform();
	return S_OK;

}

HRESULT _stdcall GDIAnalogClock::DrawMinuteHand(INT angle)
{
	this->currentWidth = static_cast<INT>(floor(double(this->bmClockPanel->GetWidth() * zoomLevel)));
	this->currentHeigth = static_cast<INT>(floor(double(this->bmClockPanel->GetHeight() * zoomLevel)));


	this->gdiGraphics->ScaleTransform(zoomLevel, zoomLevel);
	this->gdiGraphics->TranslateTransform(this->currentWidth / 2, this->currentHeigth / 2, Gdiplus::MatrixOrderAppend);
	this->gdiGraphics->RotateTransform(angle);

	this->gdiGraphics->DrawImage(this->bmHourHand, -25, -25, this->bmMinuteHand->GetWidth() / 2, this->bmClockPanel->GetHeight() / 2 - 50);
	this->gdiGraphics->ResetTransform();
	return S_OK;
}

HRESULT _stdcall GDIAnalogClock::DrawSecondHand(INT angle)
{
	this->currentWidth = static_cast<INT>(floor(double(this->bmClockPanel->GetWidth() * zoomLevel)));
	this->currentHeigth = static_cast<INT>(floor(double(this->bmClockPanel->GetHeight() * zoomLevel)));


	this->gdiGraphics->ScaleTransform(zoomLevel, zoomLevel);
	this->gdiGraphics->TranslateTransform(this->currentWidth / 2, this->currentHeigth / 2, Gdiplus::MatrixOrderAppend);
	this->gdiGraphics->RotateTransform(angle);

	this->gdiGraphics->DrawImage(this->bmSecondHand, -25, -25, this->bmSecondHand->GetWidth() / 2, this->bmClockPanel->GetHeight() / 2 - 30);
	this->gdiGraphics->ResetTransform();
	return S_OK;
}

void _stdcall GDIAnalogClock::BeginDraw()
{
	this->coreHdc = GetDC(nullptr);
	this->gdiDrawingDC = CreateCompatibleDC(this->coreHdc);
	this->currentBitmap = CreateCompatibleBitmap(this->coreHdc, currentWidth, currentHeigth);
	this->oldBitmap = (HBITMAP)SelectObject(this->gdiDrawingDC, this->currentBitmap);

	this->gdiGraphics = new Gdiplus::Graphics(this->gdiDrawingDC);
	this->gdiGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	this->gdiGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
}

void _stdcall GDIAnalogClock::EndDraw()
{
	SelectObject(this->gdiDrawingDC, oldBitmap);
	DeleteObject(this->currentBitmap);
	DeleteDC(this->gdiDrawingDC);
	ReleaseDC(nullptr, this->coreHdc);
	delete this->gdiGraphics;
}

void _stdcall GDIAnalogClock::Update(BYTE alpha)
{
	BLENDFUNCTION blend;
	ZeroMemory(&blend, sizeof(BLENDFUNCTION));
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = alpha;
	blend.AlphaFormat = AC_SRC_ALPHA;

	RECT hWndCoord;
	ZeroMemory(&hWndCoord, sizeof(RECT));
	GetWindowRect(this->corehWnd, &hWndCoord);

	this->currentWidth = static_cast<INT>(floor(double(this->bmClockPanel->GetWidth() * zoomLevel)));
	this->currentHeigth = static_cast<INT>(floor(double(this->bmClockPanel->GetHeight() * zoomLevel)));

	Gdiplus::Point wndPosition = {hWndCoord.left, hWndCoord.top};
	Gdiplus::Size wndSize(currentWidth, currentHeigth);
	Gdiplus::Point imagePosition = { 0, 0 };

	UpdateLayeredWindow(this->corehWnd,
		this->coreHdc,
		(POINT*)&wndPosition,
		(SIZE*)&wndSize,
		this->gdiDrawingDC,
		(POINT*)&imagePosition,
		0,
		&blend,
		ULW_ALPHA
	);

}

void _stdcall GDIAnalogClock::Begin(HINSTANCE hInstance, WNDPROC WndProc)
{
	this->currentWidth = static_cast<int>(floor(double(this->bmClockPanel->GetWidth() * zoomLevel)));
	this->currentHeigth = static_cast<int>(floor(double(this->bmClockPanel->GetHeight() * zoomLevel)));

	this->sysIcon = (HICON)LoadImage(NULL,
		L"clock.ico",
		IMAGE_ICON,
		0,
		0,
		LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED
	);

	ntfData.cbSize = sizeof(NOTIFYICONDATA);
	ntfData.hWnd = this->corehWnd;
	ntfData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	ntfData.hIcon = sysIcon;
	ntfData.uCallbackMessage = WM_USER_CLOCK_SHELL;

	Shell_NotifyIcon(NIM_ADD, &ntfData);

	SetWindowLongPtr(this->corehWnd, GWLP_USERDATA, (LONG_PTR)this);
	SetTimer(this->corehWnd, IDT_ABSOLUTETIMER, 1000, (TIMERPROC)&GDIWindowProcedure);

	this->DrawClock(this->currentAlpha);
	this->Start();
}

void _stdcall GDIAnalogClock::DrawClock(INT alpha)
{
	this->BeginDraw();
	SYSTEMTIME procedureTime;
	GetSystemTime(&procedureTime);
	if (procedureTime.wHour > 12)
		procedureTime.wHour = procedureTime.wHour - 12;
	DOUBLE hourAngle = procedureTime.wHour * 30 - 90 + (DOUBLE)(procedureTime.wMinute * 0.5f) ;
	INT minuteAngle = procedureTime.wMinute * 6 + 180;
	INT secondAngle = procedureTime.wSecond * 6 + 180;

	this->DrawClockPanel();
	this->DrawHourHand(hourAngle);
	this->DrawMinuteHand(minuteAngle);
	this->DrawSecondHand(secondAngle);
	this->Update(alpha);
	this->EndDraw();
}
inline void _stdcall GDIAnalogClock::SetWindowState(BOOL state)
{
	this->isWindowVisible = state;
}

inline void _stdcall GDIAnalogClock::SetAlpha(INT newAlpha)
{
	this->currentAlpha = newAlpha;
}

inline HWND _stdcall GDIAnalogClock::GetWindID()
{
	return this->corehWnd;
}

inline INT _stdcall GDIAnalogClock::GetAlpha()
{
	return this->currentAlpha;
}

inline INT _stdcall GDIAnalogClock::GetWindowState()
{
	return this->isWindowVisible;
}

GDIAnalogClock::~GDIAnalogClock()
{
	if(this->bmClockPanel)
		delete this->bmClockPanel;
	if(this->bmHourHand)
		delete this->bmHourHand;
	if(this->bmMinuteHand)
		delete this->bmMinuteHand;
	if(this->bmSecondHand)
		delete this->bmSecondHand;

	Shell_NotifyIcon(NIM_DELETE, &ntfData);
}