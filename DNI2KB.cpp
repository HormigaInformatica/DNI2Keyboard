
#include "stdafx.h"
#include "DNI2KB.h"
#include "cryptoki230/cryptoki.h"
#include <string>

#define MAX_LOADSTRING 100
#define	WM_USER_SHELLICON WM_USER + 1
using namespace std;

// Global Variables:
HINSTANCE hInst;	// current instance
NOTIFYICONDATA nidApp;
HMENU hPopMenu;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szApplicationToolTip[MAX_LOADSTRING];	    // the main window class name
BOOL bDisable = FALSE;							// keep application state

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
bool Init();
string tohex(const char* data, int size);
string fromhex(const string& hex);
void sendkey(int ky);


DWORD WINAPI DNI2KB(LPVOID lpParameter);


int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DNI2KB, szWindowClass, MAX_LOADSTRING);
	
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DNI2KB));

	DWORD myThreadID;
	HANDLE myHandle = CreateThread(0, 0, DNI2KB, nullptr , 0, &myThreadID);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DNI2KB));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DNI2KB);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   HICON hMainIcon;
   if (!Init())return FALSE;
  

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
	  return FALSE;
   }

   hMainIcon = LoadIcon(hInstance,(LPCTSTR)MAKEINTRESOURCE(IDI_DNI2KB)); 

   nidApp.cbSize = sizeof(NOTIFYICONDATA); // sizeof the struct in bytes 
   nidApp.hWnd = (HWND) hWnd;              //handle of the window which will process this app. messages 
   nidApp.uID = IDI_DNI2KB;           //ID of the icon that willl appear in the system tray 
   nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; //ORing of all the flags 
   nidApp.hIcon = hMainIcon; // handle of the Icon to be displayed, obtained from LoadIcon 
   nidApp.uCallbackMessage = WM_USER_SHELLICON; 
   LoadString(hInstance, IDS_APPTOOLTIP,nidApp.szTip,MAX_LOADSTRING);
   Shell_NotifyIcon(NIM_ADD, &nidApp); 
  

   nidApp.uVersion = NOTIFYICON_VERSION;
   Shell_NotifyIcon(NIM_SETVERSION, &nidApp);


   nidApp.uFlags = NIF_INFO;
   _tcscpy_s(nidApp.szInfo, _T("Cuando insertes un DNI en el lector, se pulsarán los digitos del dni y la tecla ENTER"));
   _tcscpy_s(nidApp.szInfoTitle, _T("DNI 2 Keyboard"));
   nidApp.uTimeout = 30000;
   nidApp.dwInfoFlags = NIIF_INFO;
   Shell_NotifyIcon(NIM_MODIFY, &nidApp);

   return TRUE;
}

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
CK_FUNCTION_LIST_PTR p11;
CK_SESSION_HANDLE ses;
bool Init()
{
	CK_RV rv;
	

	HINSTANCE dll;
	
	try
	{	//Comprobamos si existe la carpeta temporal
#ifdef WIN64
		dll = LoadLibrary(L"c:\\Windows\\System32\\DNIe_P11_x64.dll");
#else
		dll = LoadLibrary(L"c:\\Windows\\System32\\DNIe_P11.dll");
#endif
		if (dll == NULL) {
			auto error = GetLastError();
			MessageBox(NULL, L"No se ha encontrado la ruta c:\\windows\\system32\\DNIe_P11_x64.dll \nInstale los drivers del DNI Electrónico de la página de la policia.\n\nhttps://www.dnielectronico.es/", L"DLL No encontrada", MB_ICONWARNING | MB_OK | MB_DEFBUTTON1);
			return false;
		}
		//pruizpC_GetFunctionList = (CK_RV(__cdecl *)(CK_FUNCTION_LIST_PTR_PTR ppFunctionList)) GetProcAddress(dll, "C_GetFunctionList");
		const auto pC_GetFunctionList = reinterpret_cast<CK_RV(__cdecl*)(CK_FUNCTION_LIST_PTR_PTR ppFunctionList)>(GetProcAddress(dll, "C_GetFunctionList"));
		rv = pC_GetFunctionList(&p11);
		if (rv != CKR_OK) {
			MessageBox(NULL, L"No se ha podido obtener la lista de funciones PKCS11. Está el archivo c:\\windows\\system32\\DNIe_P11_x64.dll en su sitio correcto?", L"DLL ERROR", MB_ICONWARNING | MB_OK | MB_DEFBUTTON1);
			return false;
		}

		// Inicializamos pkcs11
		rv = p11->C_Initialize(NULL_PTR);
		if (rv != CKR_OK) {
			MessageBox(NULL, L"No se ha podido inicializar la librería PKCS11.\n Tienes un lector de DNI instalado?", L"PKCS11 Init Error", MB_ICONWARNING | MB_OK | MB_DEFBUTTON1);
			return false;
		}
	}
	catch (exception e) {
		return false;
	}
	
	return true;
	// user defined message that will be sent as the notification message to the Window Procedure 
}

string dni="";
DWORD WINAPI DNI2KB(LPVOID lpParameter)
{
	while (true) {
		Sleep(1000);

		if (bDisable) continue;
		
		CK_RV rv;
		rv = p11->C_OpenSession(65536, CKF_RW_SESSION | CKF_SERIAL_SESSION, NULL, NULL, &ses);
		if (rv != CKR_OK) {
			dni = "";
			continue;
		}

		vector<CK_OBJECT_HANDLE> ids;
		CK_ULONG numfilters = (CK_ULONG)1;
		CK_ATTRIBUTE* pTemplate = (CK_ATTRIBUTE*)malloc(numfilters * sizeof(CK_ATTRIBUTE));
		CK_BYTE** pointers = (CK_BYTE**)malloc(numfilters * sizeof(CK_BYTE*));
		string value = "CertAutenticacion";
		CK_ULONG 	size = (CK_ULONG)value.size();
		pointers[0] = (CK_BYTE*)malloc(size);
		memcpy(pointers[0], value.c_str(), size);

		pTemplate[0].type = CKA_LABEL;
		pTemplate[0].ulValueLen = size;
		pTemplate[0].pValue = pointers[0];
		try {
			// iniciamos búsqueda de objetos
			rv = p11->C_FindObjectsInit(ses, pTemplate, numfilters);

			// buscamos y agregamos al vector
			while (rv == CKR_OK) {
				CK_OBJECT_HANDLE h;
				CK_ULONG count;
				rv = p11->C_FindObjects(ses, &h, 1, &count);
				if (rv != CKR_OK) break;
				if (!count) break;
				ids.push_back(h);
			}
		}
		catch (exception e) {
			
		}

		free(pointers[0]);


		//delete pointers;
		free(pointers);
		free(pTemplate);
		// cerramos y liberamos memoria
		p11->C_FindObjectsFinal(ses);
		if (ids.size() == 0) {
			continue;
		}
		if (rv != CKR_OK) continue;

		string pubcert = "";
		string par_act = "";
		for (auto& i : ids) {


			try {
				CK_RV rv;
				CK_ATTRIBUTE attr;
				attr.type = CKA_SUBJECT;
				attr.pValue = NULL;

				// obtenemos tamaño
				rv = p11->C_GetAttributeValue(ses, i, &attr, 1);
				if (rv != CKR_OK) continue;

				// no hay atributo
				if (attr.ulValueLen == 0) continue;

				// memoria
				vector<CK_BYTE> data(attr.ulValueLen); // RAII
				attr.pValue = &data[0];

				// obtenemos dato
				rv = p11->C_GetAttributeValue(ses, i, &attr, 1);
				if (rv != CKR_OK)continue;

				pubcert = tohex(reinterpret_cast<char const*>(attr.pValue), attr.ulValueLen);
				const int pos_inicio = 0;
				int pos_act = 0;
				
				//Ignoramos el tipo
				pos_act += 2;
				par_act = pubcert.substr(pos_act, 2);
				int len;
				if (par_act == "81") { pos_act += 2; par_act = pubcert.substr(pos_act, 2); }
				if (par_act == "82") {
					pos_act += 2;
					par_act = pubcert.substr(pos_act, 4);
					len = std::stoul(par_act, nullptr, 16) * 2;
					pos_act += 2;
				}
				else { len = std::stoul(par_act, nullptr, 16) * 2; }
				pos_act += 4;
				par_act = pubcert.substr(pos_act, 2);

				if (par_act == "81") { pos_act += 2; par_act = pubcert.substr(pos_act, 2); }
				if (par_act == "82") {
					pos_act += 2;
					par_act = pubcert.substr(pos_act, 4);
					len = std::stoul(par_act, nullptr, 16) * 2;
					pos_act += 2;
				}
				else { len = std::stoul(par_act, nullptr, 16) * 2; }
				pos_act += len + 24;
				par_act = fromhex(pubcert.substr(pos_act, 18));
				


			}
			catch (bad_alloc&) { continue; }

		}
		rv = p11->C_CloseSession(ses);
		if (rv != CKR_OK) {
			continue;
		}
		if (par_act == dni) continue;

		dni = par_act;
		try {
			nidApp.uFlags = NIF_INFO;
			_tcscpy_s(nidApp.szInfo, s2ws(dni).c_str());
			_tcscpy_s(nidApp.szInfoTitle, _T("Enviando DNI"));
			nidApp.uTimeout = 30000;
			nidApp.dwInfoFlags = NIIF_INFO;
			Shell_NotifyIcon(NIM_MODIFY, &nidApp);

			for (auto& l : dni) {
				sendkey((int)l);
			}
			sendkey((int)10);
			sendkey((int)13);
		}
		catch (...) {}
		
	}

	return 0;


}

void sendkey(int ky)
{

	try {
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0; 
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;
		ip.ki.wVk = ky; 
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));
		ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
		SendInput(1, &ip, sizeof(INPUT));

	}
	catch (...) {}
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	POINT lpClickPoint;

	switch (message)
	{

	case WM_USER_SHELLICON: 
		// systray msg callback 
		switch(LOWORD(lParam)) 
		{   
			case WM_RBUTTONDOWN: 
				UINT uFlag = MF_BYPOSITION|MF_STRING;
				GetCursorPos(&lpClickPoint);
				hPopMenu = CreatePopupMenu();
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ABOUT,_T("Acerca de..."));
				if ( bDisable == TRUE )
				{
					uFlag |= MF_GRAYED;
				}
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_SEPARATOR,IDM_SEP,_T("SEP"));				
				if ( bDisable == TRUE )
				{
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ENABLE,_T("Habilitar"));									
				}
				else 
				{
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_DISABLE,_T("Deshabilitar"));				
				}
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_SEPARATOR,IDM_SEP,_T("SEP"));				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_EXIT,_T("Cerrar"));
									
				SetForegroundWindow(hWnd);
				TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hWnd,NULL);
				return TRUE; 

		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_DISABLE:
				bDisable = TRUE;
				break;
			case IDM_ENABLE:
				bDisable = FALSE;
				break;
			case IDM_EXIT:
				Shell_NotifyIcon(NIM_DELETE,&nidApp);
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

string tohex(const char* data, int size)
{
	stringstream out;

	for (int i = 0; i < size; i++) {
		unsigned char c = data[i];
		out << setfill('0') << setw(2) << std::hex << uppercase << (int)c;
	}
	return out.str();
}


string fromhex(const string& hex)
{
	int p = -1;
	stringstream out;

	for (unsigned int i = 0; i < hex.size(); i++)
	{
		unsigned char c = hex.data()[i];
		int n = -1;

		if (c >= '0' && c <= '9') n = c - '0';
		if (c >= 'A' && c <= 'F') n = c - 'A' + 10;
		if (c >= 'a' && c <= 'f') n = c - 'a' + 10;

		if (n >= 0) {
			if (p < 0) {
				p = n;
			}
			else {
				c = (p * 16 + n);
				out << c;
				p = -1;
			}
		}
	}

	return out.str();
}