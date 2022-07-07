#include "GDIAnalogClock.hpp"
#include <ShObjIdl.h>

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	HRESULT hr = S_OK;
	ITaskbarList* pTaskbarList = NULL;
	GDIAnalogClock* gdiClock = NULL;

	hr = CoInitialize(0);
	if (SUCCEEDED(hr))
		hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (LPVOID*)&pTaskbarList);

	if (SUCCEEDED(hr))
	{
		gdiClock = new GDIAnalogClock();
		hr = gdiClock->Init_Load(TEXT("sample.png"),
			TEXT("minutar.png"),
			TEXT("minutar.png"),
			TEXT("secundar.png"), hInstance);

		gdiClock->SetWindowPosition();
		pTaskbarList->DeleteTab(gdiClock->GetWindID());
		gdiClock->Begin(hInstance);
	}

	pTaskbarList->Release();
	CoUninitialize();

	delete gdiClock;
	return 0;
}
