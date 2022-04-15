#include <windows.h>
#include "resource.h"
#pragma warning (disable:4996)
#define MAX_LEN 10000

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void setCaption();
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("Windows 메모장");
HWND mainhWnd;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	HACCEL hAccel;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	mainhWnd = hWnd;
	setCaption();
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	while (GetMessage(&Message, NULL, 0, 0)) {
		if (!TranslateAccelerator(hWnd, hAccel, &Message)) {
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}
	return (int)Message.wParam;
}

HWND hEdit;
RECT windows;
TCHAR *defaultFileName = TEXT("제목 없음.txt");
TCHAR fileName[256];
TCHAR path[256] = TEXT("*.txt"); //파일의 저장 경로
TCHAR bPath[256]; //백업 경로
CHOOSEFONT CFT;
COLORREF Col;
UINT FRMsg;
HWND hDlgFR = NULL;
FINDREPLACE FR;
TCHAR szFindWhat[MAX_LEN];
TCHAR szReplaceWhat[MAX_LEN];
FINDREPLACE *pFR;
OPENFILENAME OFN;
LOGFONT lf;
HFONT hf;
TCHAR backUp[MAX_LEN];
bool isSaved = false;
bool isEdited = false;
bool isWrap = false;
bool findDown = false;
bool findMatch = false;
bool findLower = false;
#define ID_EDIT 101

void parsingFileName() {
	TCHAR tmp[256];
	int index = 0;
	int j = 0;
	for (int i = 0; i < lstrlen(path); i++) {
		if (path[i] == '\\') {
			index = i;
		}
	}
	for (int i = index + 1; i < lstrlen(path); i++) {
		tmp[j] = path[i];
		j++;
	}
	tmp[j] = '\0';
	lstrcpy(fileName, tmp);
}

void setCaption() {
	TCHAR tmp[256];
	if (isSaved) {
		parsingFileName();
	}
	else {
		lstrcpy(fileName, defaultFileName);
	}
	if (isEdited) {
		wsprintf(tmp, TEXT("*%s - %s"), fileName, lpszClass);
	}
	else {
		wsprintf(tmp, TEXT("%s - %s"), fileName, lpszClass);
	}
	SetWindowText(mainhWnd, tmp);
}

void undo() {
	TCHAR str1[MAX_LEN];
	GetWindowText(hEdit, str1, MAX_LEN);
	SetWindowText(hEdit, backUp);
	lstrcpy(backUp, str1);
	SendMessage(hEdit, EM_SETSEL, 0, -1);
}

void saveFile() {
	HANDLE hFile;
	DWORD dwTemp;
	TCHAR str[MAX_LEN];
	char strUtf8[MAX_LEN] = { 0, };
	if (!isSaved) {
		lstrcpy(bPath, path);
		lstrcpy(path, defaultFileName);
		memset(&OFN, 0, sizeof(OPENFILENAME));
		OFN.lStructSize = sizeof(OPENFILENAME);
		OFN.hwndOwner = mainhWnd;
		OFN.lpstrFilter = TEXT("텍스트 파일(*.txt)\0*.txt");
		OFN.lpstrFile = path;
		OFN.nMaxFile = MAX_PATH;

		if (GetSaveFileName(&OFN) != 0) {
			char strUtf8[MAX_LEN] = { 0, };
			GetWindowText(hEdit, str, MAX_PATH);
			int nLen = WideCharToMultiByte(CP_UTF8, 0, str, lstrlenW(str), NULL, 0, NULL, NULL);
			WideCharToMultiByte(CP_UTF8, 0, str, lstrlenW(str), strUtf8, nLen, NULL, NULL);

			hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile(hFile, strUtf8, sizeof(strUtf8), &dwTemp, NULL);
			CloseHandle(hFile);
			isEdited = false;
			isSaved = true;
		}
		if (lstrcmp(OFN.lpstrFile, defaultFileName) == 0) { //취소를 누른다면 동일할 것
			lstrcpy(path, bPath);
		}
		setCaption();
	}
	else {
		GetWindowText(hEdit, str, MAX_PATH);
		int nLen = WideCharToMultiByte(CP_UTF8, 0, str, lstrlenW(str), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, str, lstrlenW(str), strUtf8, nLen, NULL, NULL);

		hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hFile, strUtf8, sizeof(strUtf8), &dwTemp, NULL);
		CloseHandle(hFile);
		isEdited = false;
		isSaved = true;
		setCaption();
	}
}

void otherSave() {
	HANDLE hFile;
	DWORD dwTemp;
	TCHAR str[MAX_LEN];
	char strUtf8[MAX_LEN] = { 0, };
	lstrcpy(bPath, path);
	lstrcpy(path, TEXT("제목 없음.txt"));
	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = mainhWnd;
	OFN.lpstrFilter = TEXT("텍스트 파일(*.txt)\0*.txt");
	OFN.lpstrFile = path;
	OFN.nMaxFile = MAX_PATH;

	if (GetSaveFileName(&OFN) != 0) {
		char strUtf8[MAX_LEN] = { 0, };
		GetWindowText(hEdit, str, MAX_PATH);
		int nLen = WideCharToMultiByte(CP_UTF8, 0, str, lstrlenW(str), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, str, lstrlenW(str), strUtf8, nLen, NULL, NULL);

		hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hFile, strUtf8, sizeof(strUtf8), &dwTemp, NULL);
		CloseHandle(hFile);
		isEdited = false;
		isSaved = true;
	}
	if (lstrcmp(OFN.lpstrFile, defaultFileName) == 0) { //취소를 누른다면 동일할 것
		lstrcpy(path, bPath);
	}
	setCaption();
}

void setLower(TCHAR* tchar) {
	for (int i = 0; i < lstrlen(tchar); i++) {
		if (tchar[i] >= 'a' && tchar[i] <= 'z') {
			tchar[i] -= 32;
		}
		else {
			tchar[i] = tchar[i];
		}
	}
}

void find(){ //찾을 단어 szFindWhat
	int start, end;
	bool finded = false;
	TCHAR str[MAX_LEN];
	TCHAR find[MAX_LEN];
	start = 0;
	GetWindowText(hEdit, str, MAX_LEN);
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
	if (findLower) {
		lstrcpy(find, szFindWhat);
	}
	else {
		lstrcpy(find, szFindWhat);
		setLower(find);
		setLower(str);
	}
	if (start != end) {
		start--;
	}
	if (findDown) {
		for (int i = end; i < lstrlen(str); i++) {
			if (str[i] == find[0]) { //첫글자가 같다면 비교 시작
				for (int j = 0; j < lstrlen(find); j++) {
					if (str[i + j] != find[j]) {
						finded = false;
						break;
					}
					if (j == lstrlen(find) - 1) {
						finded = true;
						break;
					}
				}
			}
			if (finded) {
				SetFocus(hEdit);
				SendMessage(hEdit, EM_SETSEL, i, lstrlen(szFindWhat) + i);
				break;
			}
		}
	}
	else {
		for (int i = start; i > -1; i--) {
			if (str[i] == find[lstrlen(find) - 1]) { //마지막 글자가 같다면 비교 시작
				for (int j = 0; j < lstrlen(find); j++) {
					if (str[i - lstrlen(find) + j + 1] != find[j]) {
						finded = false;
						break;
					}
					if (j == lstrlen(find) - 1) {
						finded = true;
						break;
					}
				}
			}
			if (finded) {
				SetFocus(hEdit);
				SendMessage(hEdit, EM_SETSEL, i - lstrlen(szFindWhat) + 1, i + 1);
				break;
			}
		}
	}
	if (!finded) {
		wsprintf(str, TEXT("\"%s\"을(를) 찾을 수 없습니다."), szFindWhat);
		MessageBox(mainhWnd, str, lpszClass, MB_OK | MB_ICONINFORMATION);
	}
}

void replace() {
	bool finded = false;
	int start, end;
	TCHAR str[MAX_LEN];
	TCHAR find[MAX_LEN];
	GetWindowText(hEdit, str, MAX_LEN);
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
	if (findLower) {
		lstrcpy(find, szFindWhat);
	}
	else {
		lstrcpy(find, szFindWhat);
		setLower(find);
		setLower(str);
	}
	if (start != end) {
		start--;
	}
	for (int i = start; i < lstrlen(str); i++) {
		if (str[i] == find[0]) { //첫글자가 같다면 비교 시작
			for (int j = 0; j < lstrlen(find); j++) {
				if (str[i + j] != find[j]) {
					finded = false;
					break;
				}
				if (j == lstrlen(find) - 1) {
					finded = true;
					break;
				}
			}
		}
		if (finded) {
			SetFocus(hEdit);
			SendMessage(hEdit, EM_SETSEL, i - lstrlen(szFindWhat) + 1, i + 1); //역방향
			SendMessage(hEdit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szReplaceWhat);
			SendMessage(hEdit, EM_SETSEL, i - lstrlen(szFindWhat) + 1, i + 1);
			break;
		}
	}
	if (!finded) {
		wsprintf(str, TEXT("\"%s\"을(를) 찾을 수 없습니다."), szFindWhat);
		MessageBox(mainhWnd, str, lpszClass, MB_OK | MB_ICONINFORMATION);
	}
}

void replaceAll() {
	int cnt = 0;
	int start, end;
	bool finded = false;
	TCHAR str[MAX_LEN];
	TCHAR find[MAX_LEN];
	while (true) {
		finded = false;
		start = 0;
		if (findLower) {
			lstrcpy(find, szFindWhat);
		}
		else {
			lstrcpy(find, szFindWhat);
			setLower(find);
			setLower(str);
		}
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
		if (start != end) {
			start--;
		}
		GetWindowText(hEdit, str, MAX_LEN);
		for (int i = 0; i < lstrlen(str); i++) {
			if (str[i] == find[0]) { //첫글자가 같다면 비교 시작
				for (int j = 0; j < lstrlen(szFindWhat); j++) {
					if (str[i + j] != find[j]) {
						finded = false;
						break;
					}
					if (j == lstrlen(szFindWhat) - 1) {
						finded = true;
						break;
					}
				}
			}
			if (finded) {
				SetFocus(hEdit);
				SendMessage(hEdit, EM_SETSEL, i, lstrlen(szFindWhat) + i);
				SendMessage(hEdit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szReplaceWhat);
				cnt++;
				break;
			}
		}
		if (!finded) {
			if (cnt == 0) {
				wsprintf(str, TEXT("\"%s\"을(를) 찾을 수 없습니다."), szFindWhat);
				MessageBox(mainhWnd, str, lpszClass, MB_OK | MB_ICONINFORMATION);
			}
			break;
		}
	}
}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, IDOK);
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	static DWORD ThreadID;
	static HANDLE hThread;
	static SYSTEMTIME Timer;
	static TCHAR tmp[MAX_LEN];
	static int Count = 1;
	static TCHAR str[MAX_LEN];

	//모달리스 다이얼로그기 때문에 메인 프로그램과 따로 작동해야한다.
	if (iMessage == FRMsg) {
		pFR = (FINDREPLACE *)lParam;
		// 대화상자 종료 
		if (pFR->Flags & FR_DIALOGTERM) {
			hDlgFR = NULL;
			return 0;
		}
		if (pFR->Flags & FR_DOWN) {
			findDown = true;
		}
		else {
			findDown = false;
		}
		if (pFR->Flags & FR_MATCHCASE) {
			findLower = true;
		}
		else {
			findLower = false;
		}
		if (pFR->Flags & FR_WHOLEWORD) {
			MessageBox(mainhWnd, TEXT("단어 단위로 검색은 현재 Windows 메모장에서 지원하지 않는 기능입니다."), lpszClass, MB_OK);
		}
		// 다음 찾기 
		if (pFR->Flags & FR_FINDNEXT) {
			find();
		}
		else if (pFR->Flags & FR_REPLACE) {
			replace();
		}
		else if (pFR->Flags & FR_REPLACEALL) {
			replaceAll();
		}
		return 0;
	}

	switch (iMessage) {
	case WM_CREATE:
		GetClientRect(hWnd, &windows);
		hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_HSCROLL | WS_VSCROLL,
			windows.left, windows.top, windows.right, windows.bottom, hWnd, (HMENU)ID_EDIT, g_hInst, NULL);
		SetFocus(hEdit);
		lf.lfHeight = 14;
		lf.lfCharSet = HANGUL_CHARSET;
		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
		lstrcpy(lf.lfFaceName, TEXT("굴림체"));
		hf = CreateFontIndirect(&lf);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hf, TRUE);
		FRMsg = RegisterWindowMessage(FINDMSGSTRING);
		return 0;
	case WM_SIZE:
		GetClientRect(hWnd, &windows);
		MoveWindow(hEdit, windows.left, windows.top, windows.right, windows.bottom, TRUE);
		return 0;
	case WM_KEYLAST:
		GetWindowText(hEdit, backUp, MAX_LEN);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_file_new: //새로 만들기
			if (isEdited) {
				int result = MessageBox(hWnd, TEXT("변경된 내용을 저장하시겠습니까?"), TEXT("메모장"), MB_YESNOCANCEL);
				if (result == IDYES)
					saveFile();
				else if (result == IDCANCEL)
					break;
			}
			SetWindowText(hEdit, TEXT(""));
			isEdited = false;
			isSaved = false;
			setCaption();
			break;
		case ID_file_open: //열기
			if (isEdited) {
				int result = MessageBox(hWnd, TEXT("변경된 내용을 저장하시겠습니까?"), TEXT("메모장"), MB_YESNOCANCEL);
				if (result == IDYES)
					saveFile();
				else if (result == IDCANCEL)
					break;
			}
			lstrcpy(bPath, path);
			lstrcpy(path, TEXT("*.txt"));
			memset(&OFN, 0, sizeof(OPENFILENAME));
			OFN.lStructSize = sizeof(OPENFILENAME);
			OFN.hwndOwner = hWnd;
			OFN.lpstrFilter = TEXT("텍스트 파일(*.txt)\0*.txt");
			OFN.lpstrFile = path;
			OFN.nMaxFile = MAX_PATH;

			if (GetOpenFileName(&OFN) != 0) {
				HANDLE hFile;
				DWORD dwTemp;
				TCHAR tStr[MAX_LEN] = TEXT("");
				char str[MAX_LEN];
				memset(str, 0, 9999 * sizeof(char));
				hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
				ReadFile(hFile, &str, MAX_LEN, &dwTemp, NULL);
				CloseHandle(hFile);
				MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, strlen(str), tStr, strlen(str));
				SetWindowText(hEdit, tStr);
				isEdited = false;
				isSaved = true;
			}

			if (lstrcmp(OFN.lpstrFile, TEXT("*.txt")) == 0) { //취소를 누른다면 동일할 것
				lstrcpy(path, bPath);
			}
			setCaption();
			break;
		case ID_file_save: //저장
			saveFile();
			break;
		case ID_file_other_name_save: //다른 이름으로 저장
			otherSave();
			break;
		case ID_file_exit: //끝내기
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case ID_edit_undo: //실행 취소
			undo();
			break;
		case ID_edit_cut: //잘라내기
			SendMessage(hEdit, WM_CUT, 0, 0);
			break;
		case ID_edit_copy: //복사
			SendMessage(hEdit, WM_COPY, 0, 0);
			break;
		case ID_edit_paste: //붙여넣기
			SendMessage(hEdit, WM_PASTE, 0, 0);
			break;
		case ID_edit_delete: //삭제
			SendMessage(hEdit, WM_CLEAR, 0, 0);
			break;
		case ID_edit_find: //찾기
			if (hDlgFR == NULL) {
				memset(&FR, 0, sizeof(FINDREPLACE));
				FR.lStructSize = sizeof(FINDREPLACE);
				FR.hwndOwner = hWnd;
				FR.lpstrFindWhat = szFindWhat;
				FR.wFindWhatLen = MAX_LEN;
				FR.Flags = FR_DOWN;

				hDlgFR = FindText(&FR);
			}
			break;
		case ID_edit_change: //바꾸기
			if (hDlgFR == NULL) {
				memset(&FR, 0, sizeof(FINDREPLACE));
				FR.lStructSize = sizeof(FINDREPLACE);
				FR.hwndOwner = hWnd;
				FR.lpstrFindWhat = szFindWhat;
				FR.lpstrReplaceWith = szReplaceWhat;
				FR.wFindWhatLen = MAX_LEN;
				FR.wReplaceWithLen = MAX_LEN;
				FR.Flags = FR_DOWN;
				hDlgFR = ReplaceText(&FR);
			}
			break;
		case ID_edit_all: //모두 선택
			SendMessage(hEdit, EM_SETSEL, 0, -1);
			break;
		case ID_edit_date: //날짜,시간
			GetLocalTime(&Timer);
			TCHAR time[256];
			TCHAR str1[MAX_LEN];
			wsprintf(time, TEXT(" %d시 %d분 %d-%d-%d"), Timer.wHour, Timer.wMinute, Timer.wYear, Timer.wMonth, Timer.wDay);
			GetWindowText(hEdit, str1, MAX_LEN);
			lstrcpy(backUp, str1);
			lstrcat(str1, time);
			SetWindowText(hEdit, str1);
			SendMessage(hEdit, EM_SETSEL, lstrlen(str1), lstrlen(str1));
			break;
		case ID_font: //글씨체
			memset(&CFT, 0, sizeof(CHOOSEFONT));
			CFT.lStructSize = sizeof(CHOOSEFONT);
			CFT.hwndOwner = hWnd;
			CFT.lpLogFont = &lf;
			CFT.Flags = CF_EFFECTS | CF_SCREENFONTS;
			if (ChooseFont(&CFT)) {
				hf = CreateFontIndirect(&lf);
				Col = CFT.rgbColors;
				SendMessage(hEdit, WM_SETFONT, (WPARAM)hf, TRUE);
			}
			break;
		case ID_help: //메모장 정보
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, AboutDlgProc);
			break;
		case ID_EDIT:
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
				if (!isEdited) {
					isEdited = true;
					setCaption();
					break;
				}
				return 0;
			case EN_ERRSPACE:
				MessageBox(mainhWnd, TEXT("입력 한도를 초과하였습니다!"), lpszClass, MB_OK | MB_ICONWARNING);
				SetWindowText(hEdit, backUp);
			}
		}
		return 0;
	case WM_CTLCOLOREDIT:
		if ((HWND)lParam == GetDlgItem(hWnd, ID_EDIT)) {
			SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
			SetTextColor((HDC)wParam, Col);
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		}
		return 0;
	case WM_CLOSE:
		if (isEdited) {
			int result = MessageBox(hWnd, TEXT("변경된 내용을 저장하시겠습니까?"), TEXT("메모장"), MB_YESNOCANCEL);
			if (result == IDYES) {
				saveFile();
			}
			else if (result == IDCANCEL){
				return 0;
			}
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}