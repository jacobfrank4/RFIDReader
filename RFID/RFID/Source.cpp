/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - An application that handles communication between 2 machines over a serial cable
--
-- PROGRAM: Assignment1: Serial Communication
--
-- FUNCTIONS:
-- void checkCommPort(WPARAM wParam);
-- void checkbps(WPARAM wParam);
-- void checknumberofbits(WPARAM wParam);
-- void checkparity(WPARAM wParam);
-- void checkstopbits(WPARAM wParam);
-- void enableReadMode(HWND &hwnd);
-- BOOL enableWriteMode(char * lpBuf);
--
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank & Justen DePourcq
--
-- PROGRAMMER: Jacob Frank & Justen DePourcq
--
-- NOTES:
-- Currently the program does not function as expected.
-- The program has connect mode which enables reading from the serial port, but does not function properly
-- When the program enters connect mode and receives a bit, it is not properly displayed to the screen
-- The program does not handle writing over the serial port
----------------------------------------------------------------------------------------------------------------------*/

#define STRICT

#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include "Header.h"
#define READ_TIMEOUT      500  
void checkCommPort(WPARAM wParam);
void checkbps(WPARAM wParam);
void checknumberofbits(WPARAM wParam);
void checkparity(WPARAM wParam);
void checkstopbits(WPARAM wParam);
void enableReadMode(HWND &hwnd);
BOOL enableWriteMode(char * lpBuf);
VOID SuccessfulRead(char lpBuf[], DWORD dwrd, LPVOID lpParam);
DWORD WINAPI Read(LPVOID lpParam);

TCHAR Name[] = TEXT("Communication Terminal");
char str[255] = ""; //output buffer
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#pragma warning (disable: 4096)
HANDLE hndThread; // Handle for the thread

LPCSTR	lpszCommName;
COMMCONFIG	cc;
HANDLE hComm;
LPVOID buffer;

OVERLAPPED lpOverlappedwrite;

static HMENU hmenu;
bool isConnectMode;
int X = 0; int Y = 0;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMAIN
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- NOTES:
-- Initializes and creates the window and menu
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow) {

	HWND hwnd;
	MSG Msg;
	WNDCLASSEX Wcl;
	isConnectMode = false;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; //Use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW); //Cursor Style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("OPTIONMENU"); //menu class
	Wcl.cbClsExtra = 0; // no extra memory needed
	Wcl.cbWndExtra = 0;

	//Register the class
	if (!RegisterClassEx(&Wcl))
		return 0;

	//Create and display window
	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10, 600, 400, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	hmenu = GetMenu(hwnd);

	//Setup default communication parameters:
	//COM 3
	//BAUD rate: 9600bps
	//Number of bits: 8
	//Parity: None
	//Number of stop bits: 1
	checkCommPort(IDM_COM3);
	checkbps(IDM_BPS9600);
	checknumberofbits(IDM_NOB8);
	checkparity(IDM_NONE);
	checkstopbits(IDM_STOPBIT1);


	while (GetMessage(&Msg, NULL, 0, 0)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- NOTES:
-- Handles all windows events
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

	int response;
	HDC hdc;
	PAINTSTRUCT paintstruct;
	TEXTMETRIC tm;
	static unsigned k = 0;

	switch (Message) {

	case WM_CHAR:	// Process keystroke

		if (wParam == VK_ESCAPE) {
			response = MessageBox(hwnd, TEXT("Entering Command Mode"), TEXT("Enter Command Mode"), MB_OK);
			CloseHandle(hComm);
			TerminateThread(hndThread, 0);
		}
		//if (isConnectMode) {
		hdc = GetDC(hwnd);			 // get device context
		sprintf_s(str, "%c", (char)wParam); // Convert char to string
		enableWriteMode(str);
		//k++;
		//enableWriteMode
		//WriteFile(hComm, buffer, 1, NULL, &lpOverlappedwrite);
		ReleaseDC(hwnd, hdc); // Release device context
							  //}
		break;

	case WM_PAINT:		// Process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
		TextOut(hdc, 0, 0, str, strlen(str)); // output character
		EndPaint(hwnd, &paintstruct); // Release DC
		break;

		//handles menu events
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDM_COM1:
		case IDM_COM2:
		case IDM_COM3:
		case IDM_COM4:
			checkCommPort(wParam);
			break;
		case IDM_BPS110:
		case IDM_BPS300:
		case IDM_BPS600:
		case IDM_BPS1200:
		case IDM_BPS2400:
		case IDM_BPS4800:
		case IDM_BPS9600:
		case IDM_BPS14400:
		case IDM_BPS19200:
		case IDM_BPS38400:
		case IDM_BPS57600:
		case IDM_BPS115200:
		case IDM_BPS128000:
		case IDM_BPS256000:
			checkbps(wParam);
			break;
		case IDM_NOB5:
		case IDM_NOB6:
		case IDM_NOB7:
		case IDM_NOB8:
			checknumberofbits(wParam);
			break;
		case IDM_EVEN:
		case IDM_ODD:
		case IDM_NONE:
		case IDM_MARK:
		case IDM_SPACE:
			checkparity(wParam);
			break;
		case IDM_STOPBIT1:
		case IDM_STOPBIT15:
		case IDM_STOPBIT2:
			checkstopbits(wParam);
			break;
		case IDM_CONNECT:
			CheckMenuRadioItem(hmenu, IDM_READ, IDM_WRITE, IDM_CONNECT, MF_BYCOMMAND);

			//Opens the comm port for reading and writing
			if ((hComm = CreateFile(lpszCommName,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				NULL)) == INVALID_HANDLE_VALUE) {
				MessageBox(NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
				return FALSE;
			}

			if (!SetupComm(hComm, 1024, 1024)) {
				GetLastError();
			}

			if (!SetCommState(hComm, &cc.dcb)) {
				MessageBox(NULL, "ERROR SETTING UP THE COMM PORT", "", MB_OK);
				return false;
			}

			hndThread = CreateThread(NULL,
				0,
				Read,
				(LPVOID)hwnd,
				0,
				NULL);

			isConnectMode = true; //Enables Read/Write Mode

			break;
		case IDM_READ:
			CheckMenuRadioItem(hmenu, IDM_READ, IDM_WRITE, IDM_READ, MF_BYCOMMAND);

			//Opens the comm port for reading and writing
			if ((hComm = CreateFile(lpszCommName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE) {
				MessageBox(NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
				return FALSE;
			}

			if (!SetupComm(hComm, 1024, 1024)) {
				GetLastError();
			}

			if (!SetCommState(hComm, &cc.dcb)) {
				MessageBox(NULL, "ERROR SETTING UP THE COMM PORT", "", MB_OK);
				return false;
			}

			isConnectMode = true; //Enables Read/Write Mode
			hndThread = CreateThread(NULL,
				0,
				Read,
				(LPVOID)hwnd,
				0,
				NULL);
			enableReadMode(hwnd); // Enters Read mode
			break;


		case IDM_WRITE:

			//Opens the comm port for reading and writing
			if ((hComm = CreateFile(lpszCommName,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				NULL)) == INVALID_HANDLE_VALUE) {
				MessageBox(NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
				return FALSE;
			}

			if (!SetCommState(hComm, &cc.dcb)) {
				MessageBox(NULL, "ERROR SETTING UP THE COMM PORT", "", MB_OK);
				return false;
			}
			else {
				CheckMenuRadioItem(hmenu, IDM_READ, IDM_WRITE, IDM_WRITE, MF_BYCOMMAND);
			}
		}
		break;

	case WM_DESTROY: //Terminate program
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Read
--
-- DATE: October 5th, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Justen DePourcq
--
-- PROGRAMMER: Justen DePourcq
--
-- INTERFACE: DWORD WINAPI Read(LPVOID lpParam)
--
-- RETURNS: DWORD
--
-- NOTES:
-- This function uses the ReadFile function in order to read characters being sent through the serial
-- port from another computer.
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI Read(LPVOID lpParam) {
	DWORD dwRead;
	BOOL fWaitingOnRead = FALSE;
	OVERLAPPED osReader = { 0 };
	char lpBuf[1] = { 0 };

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (osReader.hEvent == NULL) {
		OutputDebugStringA("Error creating overlapped event");// Error creating overlapped event; abort.
	}

	while (1) {
		if (!fWaitingOnRead) {
			// Issue read operation.
			if (!ReadFile(hComm, lpBuf, sizeof(lpBuf), &dwRead, &osReader)) {
				if (GetLastError() != ERROR_IO_PENDING) {     // read not delayed?
					OutputDebugStringA("Error in communications 1");		// Error in communications; report it.
				}
				else {
					fWaitingOnRead = TRUE;
				}
			}
			else {
				// read completed immediately
				SuccessfulRead(lpBuf, dwRead, lpParam);
			}
		}

#define READ_TIMEOUT      500      // milliseconds

		DWORD dwRes;

		if (fWaitingOnRead) {
			dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
			switch (dwRes)
			{
				// Read completed.
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)) {
					OutputDebugStringA("Error in communications 2");
					// Error in communications; report it.
				}
				else {
					// Read completed successfully.
					SuccessfulRead(lpBuf, dwRead, lpParam);
				}

				//  Reset flag so that another opertion can be issued.
				fWaitingOnRead = FALSE;
				break;

			case WAIT_TIMEOUT:
				// Operation isn't complete yet. fWaitingOnRead flag isn't
				// changed since I'll loop back around, and I don't want
				// to issue another read until the first one finishes.
				//
				// This is a good time to do some background work.
				break;

			default:
				// Error in the WaitForSingleObject; abort.
				// This indicates a problem with the OVERLAPPED structure's
				// event handle.
				break;
			}
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: SuccessfulRead
--
-- DATE: October 5th, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Justen DePourcq
--
-- PROGRAMMER: Justen DePourcq
--
-- INTERFACE: SuccessfulRead(char lpBuf[], DWORD dwrd, LPVOID lpParam)
--
-- RETURNS: VOID
--
-- NOTES:
-- This function handles a succesfull read from the Read function and prints it to the CommShell
----------------------------------------------------------------------------------------------------------------------*/

VOID SuccessfulRead(char lpBuf[], DWORD dwrd, LPVOID lpParam) {
	HWND handle = (HWND)lpParam;
	TEXTMETRIC tm;
	HDC hdc = GetDC(handle);


	GetTextMetrics(hdc, &tm);


	if (dwrd > 0) {
		if (X * 10 > 600) {
			Y = Y + tm.tmHeight + tm.tmExternalLeading;
			X = 0;
		}

		sprintf_s(str, "%c", lpBuf[0]);
		TextOut(hdc, 10 * X, Y, str, strlen(str));
		X++;
		ReleaseDC(handle, hdc);
	}
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: enableWriteMode
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- RETURNS: Bool.
--
-- NOTES:
-- Sets program into Write mode
-- Enables the sending of its through the serial port
----------------------------------------------------------------------------------------------------------------------*/
BOOL enableWriteMode(char * lpBuf) {

	OVERLAPPED osWrite = { 0 };
	DWORD dwWritten;
	BOOL fRes;

	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
		// Error creating overlapped event handle.
		return FALSE;
	// Issue write.
	if (!WriteFile(hComm, lpBuf, 1, &dwWritten, &osWrite)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			// WriteFile failed, but it isn't delayed. Report error and abort.
			fRes = FALSE;
		}
		else {
			// Write is pending.
			if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)) {
				fRes = FALSE;
			}
			else {
				// Write operation completed successfully.
				fRes = TRUE;
			}
		}
	}
	else {
		// WriteFile completed immediately.
		fRes = TRUE;
	}
	CloseHandle(osWrite.hEvent);
	return fRes;
}



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: enableReadMode
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- RETURNS: void.
--
-- NOTES:
-- Sets program into read mode
-- takes complete control of the thread and continuously listens for input from the serial port
-- When data is received it is printed to the screen
-- Currently the function repeatedly sends a single character until the program is exited
----------------------------------------------------------------------------------------------------------------------*/

void enableReadMode(HWND &hwnd) {

	HDC hdc = GetDC(hwnd);	 // get device context;
	DWORD dwRead;
	OVERLAPPED Overlappedread = { 0 };
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	static unsigned k = 0;
	static unsigned Y = 0;

	while (isConnectMode) {


		Overlappedread.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (Overlappedread.hEvent == NULL) {
			GetLastError();
		}
		if (k * 10 > 600) {
			Y = Y + tm.tmHeight + tm.tmExternalLeading;
			k = 0;
		}

		hdc = GetDC(hwnd);	 // get device context
		ReadFile(hComm, str, 1, NULL, &Overlappedread);
		TextOut(hdc, 10 * k, Y, str, strlen(str)); // output character
		k++;
		ReleaseDC(hwnd, hdc); // Release device context
	}
	CloseHandle(hComm);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkCommPort
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- RETURNS: void.
--
-- NOTES:
-- Sets comm port to be opened
----------------------------------------------------------------------------------------------------------------------*/
void checkCommPort(WPARAM wParam) {
	switch (wParam) {
	case IDM_COM1:
		CheckMenuRadioItem(hmenu, IDM_COM1, IDM_COM4, IDM_COM1, MF_BYCOMMAND);
		lpszCommName = TEXT("com1");
		break;
	case IDM_COM2:
		CheckMenuRadioItem(hmenu, IDM_COM1, IDM_COM4, IDM_COM2, MF_BYCOMMAND);
		lpszCommName = TEXT("com2");
		break;
	case IDM_COM3:
		CheckMenuRadioItem(hmenu, IDM_COM1, IDM_COM4, IDM_COM3, MF_BYCOMMAND);
		lpszCommName = TEXT("com3");
		break;
	case IDM_COM4:
		CheckMenuRadioItem(hmenu, IDM_COM1, IDM_COM4, IDM_COM4, MF_BYCOMMAND);
		lpszCommName = TEXT("com4");
		break;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkbps
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- RETURNS: void.
--
-- NOTES:
-- Sets the baud rate for serial communication
----------------------------------------------------------------------------------------------------------------------*/
void checkbps(WPARAM wParam) {
	switch (wParam) {
	case IDM_BPS110:
		CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS110, MF_BYCOMMAND);
		cc.dcb.BaudRate = CBR_110;
		break;
	case IDM_BPS300:
		CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS300, MF_BYCOMMAND);
		cc.dcb.BaudRate = CBR_300;
		break; case IDM_BPS600:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS600, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_600;
			break;
		case IDM_BPS1200:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS1200, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_1200;
			break;
		case IDM_BPS2400:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS2400, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_2400;
			break;
		case IDM_BPS4800:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS4800, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_4800;
			break;
		case IDM_BPS9600:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS9600, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_9600;
			break;
		case IDM_BPS14400:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS14400, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_14400;
			break;
		case IDM_BPS19200:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS19200, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_19200;
			break;
		case IDM_BPS38400:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS38400, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_38400;
			break;
		case IDM_BPS57600:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS57600, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_57600;
			break;
		case IDM_BPS115200:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS115200, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_115200;
			break;
		case IDM_BPS128000:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS128000, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_128000;
			break;
		case IDM_BPS256000:
			CheckMenuRadioItem(hmenu, IDM_BPS110, IDM_BPS256000, IDM_BPS256000, MF_BYCOMMAND);
			cc.dcb.BaudRate = CBR_256000;
			break;
	}
	//SetCommState(hComm, &cc.dcb);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checknumberofbits
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- RETURNS: void.
--
-- NOTES:
-- Sets the number of bits for serial communication
----------------------------------------------------------------------------------------------------------------------*/
void checknumberofbits(WPARAM wParam) {
	switch (wParam) {
	case IDM_NOB5:
		CheckMenuRadioItem(hmenu, IDM_NOB5, IDM_NOB8, IDM_NOB5, MF_BYCOMMAND);
		cc.dcb.ByteSize = 5;
		break;
	case IDM_NOB6:
		CheckMenuRadioItem(hmenu, IDM_NOB5, IDM_NOB8, IDM_NOB6, MF_BYCOMMAND);
		cc.dcb.ByteSize = 6;
		break;
	case IDM_NOB7:
		CheckMenuRadioItem(hmenu, IDM_NOB5, IDM_NOB8, IDM_NOB7, MF_BYCOMMAND);
		cc.dcb.ByteSize = 7;
		break;
	case IDM_NOB8:
		CheckMenuRadioItem(hmenu, IDM_NOB5, IDM_NOB8, IDM_NOB8, MF_BYCOMMAND);
		cc.dcb.ByteSize = 8;
		break;
	}
	//SetCommState(hComm, &cc.dcb);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkparity
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- RETURNS: void.
--
-- NOTES:
-- Sets the parity for serial communication
----------------------------------------------------------------------------------------------------------------------*/
void checkparity(WPARAM wParam) {
	switch (wParam) {
	case IDM_EVEN:
		CheckMenuRadioItem(hmenu, IDM_EVEN, IDM_SPACE, IDM_EVEN, MF_BYCOMMAND);
		cc.dcb.Parity = EVENPARITY;
		break;
	case IDM_ODD:
		CheckMenuRadioItem(hmenu, IDM_EVEN, IDM_SPACE, IDM_ODD, MF_BYCOMMAND);
		cc.dcb.Parity = ODDPARITY;
		break;
	case IDM_NONE:
		CheckMenuRadioItem(hmenu, IDM_EVEN, IDM_SPACE, IDM_NONE, MF_BYCOMMAND);
		cc.dcb.Parity = NOPARITY;
		break;
	case IDM_MARK:
		CheckMenuRadioItem(hmenu, IDM_EVEN, IDM_SPACE, IDM_MARK, MF_BYCOMMAND);
		cc.dcb.Parity = MARKPARITY;
		break;
	case IDM_SPACE:
		CheckMenuRadioItem(hmenu, IDM_EVEN, IDM_SPACE, IDM_SPACE, MF_BYCOMMAND);
		cc.dcb.Parity = SPACEPARITY;
		break;
	}
	//SetCommState(hComm, &cc.dcb);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkstopbits
--
-- DATE: October 4, 2016
--
-- DESIGNER: Jacob Frank
--
-- PROGRAMMER: Jacob Frank
--
-- RETURNS: void.
--
-- NOTES:
-- Sets the number of stop bits for serial communication
----------------------------------------------------------------------------------------------------------------------*/
void checkstopbits(WPARAM wParam) {
	switch (wParam) {
	case IDM_STOPBIT1:
		CheckMenuRadioItem(hmenu, IDM_STOPBIT1, IDM_STOPBIT2, IDM_STOPBIT1, MF_BYCOMMAND);
		cc.dcb.StopBits = ONESTOPBIT;
		break;
	case IDM_STOPBIT15:
		CheckMenuRadioItem(hmenu, IDM_STOPBIT1, IDM_STOPBIT2, IDM_STOPBIT15, MF_BYCOMMAND);
		cc.dcb.StopBits = ONE5STOPBITS;
		break;
	case IDM_STOPBIT2:
		CheckMenuRadioItem(hmenu, IDM_STOPBIT1, IDM_STOPBIT2, IDM_STOPBIT2, MF_BYCOMMAND);
		cc.dcb.StopBits = TWOSTOPBITS;
		break;
	}
	//SetCommState(hComm, &cc.dcb);
}
