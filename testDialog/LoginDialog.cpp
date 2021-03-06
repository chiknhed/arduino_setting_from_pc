// LoginDialog.cpp : implementation file
//

#include "stdafx.h"
#include "testDialog.h"
#include "LoginDialog.h"
#include "afxdialogex.h"


#include <Windows.h>
#include <wincrypt.h>

#include "http.h"
#include "testDialogDlg.h"


// CLoginDialog dialog

IMPLEMENT_DYNAMIC(CLoginDialog, CDialogEx)

CLoginDialog::CLoginDialog(CWnd* pParent /*=NULL*/, CtestDialogDlg * p)
	: CDialogEx(CLoginDialog::IDD, pParent)
	, m_bLoggedIn(false)
{
	_parent = p;
}


CLoginDialog::~CLoginDialog()
{
}

void CLoginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOGIN_USERNAME_EDIT, m_loginUsernameEdit);
	DDX_Control(pDX, IDC_LOGIN_PASSWD_EDIT, m_loginPasswordEdit);
	DDX_Control(pDX, IDC_USERNAME_EDIT, m_usernameEdit);
	DDX_Control(pDX, IDC_PASSWD_EDIT, m_passwordEdit);
	DDX_Control(pDX, IDC_PASSWD_CONFIRM_EDIT, m_passwordConfirmEdit);
	DDX_Control(pDX, IDC_FAMILY_EDIT, m_familyNameEdit);
	DDX_Control(pDX, IDC_FIRST_EDIT, m_firstNameEdit);
	DDX_Control(pDX, IDC_MAIL_EDIT, m_mailEdit);
	DDX_Control(pDX, IDC_PHONEADD_EDIT, m_phoneEdit);
	DDX_Control(pDX, IDC_PHONE_LIST, m_phoneList);
}


BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
	ON_BN_CLICKED(IDC_PHONEADD_BTN, &CLoginDialog::OnBnClickedPhoneaddBtn)
	ON_BN_CLICKED(IDC_LOGIN_BTN, &CLoginDialog::OnBnClickedLoginBtn)
	ON_BN_CLICKED(IDC_REGISTER_BTN, &CLoginDialog::OnBnClickedRegisterBtn)
	ON_COMMAND(IDCANCEL, &CLoginDialog::OnIdcancel)
	ON_COMMAND(IDOK, &CLoginDialog::OnIdok)
END_MESSAGE_MAP()


// CLoginDialog message handlers


void CLoginDialog::OnBnClickedPhoneaddBtn()
{
	// TODO: Add your control notification handler code here
	int row;
	CString phoneNumberText;

	m_phoneEdit.GetWindowText(phoneNumberText);
	m_phoneEdit.SetWindowTextW(_T(""));
	if (!IsPhoneValid(phoneNumberText)) {
		MessageBox(_T("전화번호 형식이 잘못되었습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}

	//check duplicate number
	for (row = 0; row < m_phoneList.GetCount(); row++) {
		CString entry;
		m_phoneList.GetText(row, entry);
		if (entry == phoneNumberText) {
			MessageBox(_T("전화번호가 중복되었습니다."), _T("에러"), MB_ICONERROR | MB_OK);
			return;
		}
	}
	m_phoneList.AddString(phoneNumberText);
}


bool CLoginDialog::IsPhoneValid(CString& phoneNumberStr)
{
	int i;

	for (i = 0; i < phoneNumberStr.GetLength(); i++) {
		if (phoneNumberStr[i] <= '9' && phoneNumberStr[i] >= '0')
			continue;
		if (phoneNumberStr[i] == '+')
			continue;
		if (phoneNumberStr[i] == '-')
			continue;
		return false;
	}

	return true;
}


void CLoginDialog::OnBnClickedLoginBtn()
{
	CString usernameStr;
	CString passwordStr;

	m_loginUsernameEdit.GetWindowText(usernameStr);
	m_loginPasswordEdit.GetWindowText(passwordStr);

	if (TryLogin(usernameStr, passwordStr)) {
		_parent->m_strUsername = usernameStr;
		_parent->m_strPassword = passwordStr;
		m_bLoggedIn = true;
		_parent->NextProcess(this);
	}
}


void CLoginDialog::OnBnClickedRegisterBtn()
{
	// TODO: Add your control notification handler code here
	CString usernameStr;
	CString passwordStr;
	CString passwordConfirmStr;
	CString familyNameStr;
	CString firstNameStr;
	CString emailStr;
	int i;

	m_usernameEdit.GetWindowText(usernameStr);
	m_passwordEdit.GetWindowText(passwordStr);
	m_passwordConfirmEdit.GetWindowText(passwordConfirmStr);
	m_familyNameEdit.GetWindowText(familyNameStr);
	m_firstNameEdit.GetWindowText(firstNameStr);
	m_mailEdit.GetWindowText(emailStr);

	if (usernameStr.GetLength() == 0) {
		MessageBox(_T("사용자 아이디를 입력하지 않았습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}
	if (passwordStr.GetLength() == 0) {
		MessageBox(_T("암호를 입력하지 않았습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}
	if (passwordConfirmStr.GetLength() == 0) {
		MessageBox(_T("암호확인을 입력하지 않았습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}
	if (familyNameStr.GetLength() == 0) {
		MessageBox(_T("성을 입력하지 않았습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}
	if (firstNameStr.GetLength() == 0) {
		MessageBox(_T("이름을 입력하지 않았습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}
	if (emailStr.GetLength() == 0) {
		MessageBox(_T("이메일 주소를 입력하지 않았습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}

	if (passwordStr != passwordConfirmStr) {
		MessageBox(_T("암호가 다릅니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}

	if (m_phoneList.GetCount() == 0) {
		MessageBox(_T("전화번호를 하나이상 입력해 주십시오."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}

	bool result = AddUser(usernameStr,
		passwordStr,
		familyNameStr,
		firstNameStr,
		emailStr
		);

	if (!result) {
		MessageBox(_T("사용자 등록이 실패했습니다."), _T("에러"), MB_ICONERROR | MB_OK);
		return;
	}

	for (i = 0; i < m_phoneList.GetCount(); i++) {
		CString phonenumberStr;
		m_phoneList.GetText(i, phonenumberStr);
		result = AddPhoneNumber(usernameStr, passwordStr, phonenumberStr);
		if (!result) {
			MessageBox(_T("전화번호 등록이 실패했습니다."), _T("에러"), MB_ICONERROR | MB_OK);
			return;
		}
	}

	m_bLoggedIn = true;
	_parent->m_strUsername = usernameStr;
	_parent->m_strPassword = passwordStr;
	_parent->NextProcess(this);
}


bool CLoginDialog::GetLoginHash(CString& usernameStr, CString& passwordStr, CString& hashResultStr)
{
	DWORD dwStatus = 0;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	CString hashStr;
	DWORD cbHash = 0;
	BYTE rgbHash[40];
	CHAR rgbDigits[] = "0123456789abcdef";
	CHAR utfBuffer[400];
	
	hashStr = _T("howdi") + usernameStr + passwordStr + _T("trauma");

	WideCharToMultiByte(CP_UTF8, 0, hashStr, -1, utfBuffer, 400, NULL, NULL);

	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		return false;
	}

	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
	{
		CryptReleaseContext(hProv, 0);
		return false;
	}

	if (!CryptHashData(hHash, (const BYTE *)utfBuffer, strlen(utfBuffer), NULL)) {
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		return false;
	}

	cbHash = 40;

	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
		for (DWORD i = 0; i < cbHash; i++) {
			hashResultStr += rgbDigits[rgbHash[i] >> 4];
			hashResultStr += rgbDigits[rgbHash[i] & 0xf];
		}
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return true;
}


bool CLoginDialog::TryLogin(CString& usernameStr, CString& passwordStr)
{
	CWnd* pWndRet = GetDlgItem(IDC_LOGIN_BTN);
	CString hashResultStr;
	CMemFile MemFile;
	CString returnedData;
	int retVal;
	char return_buffer[500];
	char dataBuffer[400];
	char buffer1[400], buffer2[400];
	
	GetLoginHash(usernameStr, passwordStr, hashResultStr);

	WideCharToMultiByte(CP_UTF8, 0, usernameStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(dataBuffer, "username=%s", buffer2);

	WideCharToMultiByte(CP_UTF8, 0, passwordStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&password=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	WideCharToMultiByte(CP_UTF8, 0, hashResultStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&secure_key=%s", buffer2);
	strcat_s(dataBuffer, buffer1);	

	retVal = HttpCall(_T(SERVER_ADDRESS),
		80,
		_T("/sensor/applogin/"),
		_T(""),
		dataBuffer,
		&MemFile,
		pWndRet
		);
	ULONGLONG DataLen = MemFile.GetLength();
	MemFile.SeekToBegin();
	if (DataLen < 500 - 1) {
		MemFile.Read(return_buffer, (UINT)DataLen);
		return_buffer[DataLen] = '\0';
	} else {
		MemFile.Read(return_buffer, (UINT)500);
		return_buffer[500-1] = '\0';
	}

	CString resultStr = CA2W(return_buffer);
	returnedData.ReleaseBuffer();
	if (resultStr == _T("+suck=")) {
		return true;
	}
	
	MessageBox(_T("로그인이 실패했습니다."), _T("로그인 실패"), MB_ICONERROR | MB_OK);

	return false;
}


void CLoginDialog::OnIdcancel()
{
	// TODO: Add your command handler code here
}


void CLoginDialog::OnIdok()
{
	// TODO: Add your command handler code here
}


bool CLoginDialog::AddUser(CString& usernameStr, CString& passwordStr, CString& familyNameStr, CString& firstNameStr, CString& emailStr)
{
	CWnd* pWndRet = GetDlgItem(IDC_LOGIN_BTN);
	CString hashResultStr;
	CMemFile MemFile;
	char return_buffer[500];
	CString returnedData;
	int retVal;	
	char dataBuffer[1000];
	char buffer1[400], buffer2[400];

	GetLoginHash(usernameStr, passwordStr, hashResultStr);

	WideCharToMultiByte(CP_UTF8, 0, usernameStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(dataBuffer, "username=%s", buffer2);

	WideCharToMultiByte(CP_UTF8, 0, passwordStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&password=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	WideCharToMultiByte(CP_UTF8, 0, hashResultStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&secure_key=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	WideCharToMultiByte(CP_UTF8, 0, familyNameStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&family_name=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	WideCharToMultiByte(CP_UTF8, 0, firstNameStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&first_name=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	WideCharToMultiByte(CP_UTF8, 0, emailStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&email=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	retVal = HttpCall(_T(SERVER_ADDRESS),
		80,
		_T("/sensor/appregister/"),
		_T(""),
		dataBuffer,
		&MemFile,
		pWndRet
		);
	ULONGLONG DataLen = MemFile.GetLength();
	MemFile.SeekToBegin();
	if (DataLen < 500 - 1) {
		MemFile.Read(return_buffer, (UINT)DataLen);
		return_buffer[DataLen] = '\0';
	}
	else {
		MemFile.Read(return_buffer, (UINT)500);
		return_buffer[500 - 1] = '\0';
	}

	CString resultStr = CA2W(return_buffer);
	returnedData.ReleaseBuffer();
	if (resultStr == _T("+suck=")) {
		return true;
	}

	return false;
}


bool CLoginDialog::AddPhoneNumber(CString& usernameStr, CString& passwordStr, CString& phonenumberStr)
{
	CWnd* pWndRet = GetDlgItem(IDC_LOGIN_BTN);
	CString hashResultStr;
	CMemFile MemFile;
	CString returnedData;
	int retVal;
	char return_buffer[500];
	char dataBuffer[1000];
	char buffer1[400], buffer2[400];

	GetLoginHash(usernameStr, passwordStr, hashResultStr);

	WideCharToMultiByte(CP_UTF8, 0, usernameStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(dataBuffer, "username=%s", buffer2);

	WideCharToMultiByte(CP_UTF8, 0, passwordStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&password=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	WideCharToMultiByte(CP_UTF8, 0, hashResultStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&secure_key=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	WideCharToMultiByte(CP_UTF8, 0, phonenumberStr, -1, buffer1, 400, NULL, NULL);
	encode_char_str(buffer1, buffer2);
	sprintf_s(buffer1, "&phone_number=%s", buffer2);
	strcat_s(dataBuffer, buffer1);

	retVal = HttpCall(_T(SERVER_ADDRESS),
		80,
		_T("/sensor/appphone/"),
		_T(""),
		dataBuffer,
		&MemFile,
		pWndRet
		);
	ULONGLONG DataLen = MemFile.GetLength();
	MemFile.SeekToBegin();
	if (DataLen < 500 - 1) {
		MemFile.Read(return_buffer, (UINT)DataLen);
		return_buffer[DataLen] = '\0';
	}
	else {
		MemFile.Read(return_buffer, (UINT)500);
		return_buffer[500 - 1] = '\0';
	}

	CString resultStr = CA2W(return_buffer);
	returnedData.ReleaseBuffer();
	if (resultStr == _T("+suck=")) {
		return true;
	}

	return false;
}
