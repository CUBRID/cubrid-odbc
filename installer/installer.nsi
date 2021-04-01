;
; CUBRID ODBC installer
;   Installer design key point
;	1. Install 32bit driver only in 32bit Windows
;	2. Install both 32bit and 64bit driver in 64bit Windows
;	3. Do not ask user to choose 32bit or 64bit. to prevent confusion for beginner user


;--------------------------------
;Include Modern UI

	!include "MUI2.nsh"
	!include "x64.nsh"


;--------------------------------
;General

	;Name and file
	Name "CUBRID ODBC Driver (32/64bit)"
	OutFile "cubrid-odbc.exe"

	;Default installation folder
	InstallDir "$PROGRAMFILES\cubrid-odbc"
	
	;Get installation folder from registry if available
	InstallDirRegKey HKLM "SOFTWARE\cubrid-odbc" ""

	;Request application privileges for Windows Vista
	RequestExecutionLevel admin

;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_LICENSE "license.txt"
	!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES
	
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
	
;--------------------------------
;Languages
 
	!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Function .onInit
  Call CheckVCRedist
    Pop $R9
    StrCmp $R9 "No" 0 +4
    MessageBox MB_OK|MB_ICONSTOP "This driver requires Visual Studio(2015 or Higher) x86 Redistributable. Please install the Redistributable then run this installer again." IDOK ERRORVCRedist
  ERRORVCRedist:
  Abort
FunctionEnd

Section "ODBC Driver" SecODBC

	SetOutPath "$INSTDIR"
	
	;Store installation folder
	WriteRegStr HKLM "SOFTWARE\cubrid-odbc" "" $INSTDIR
	
	;Create uninstaller
	WriteUninstaller "$INSTDIR\uninstall.exe"

	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\cubrid-odbc" "DisplayName" "CUBRID ODBC Driver 32/64bit"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\cubrid-odbc" "UninstallString" "$INSTDIR\uninstall.exe"

	; add 32bit driver file and register to normal registry
	File "README.txt"
  Call WriteRegeDit32
	${If} ${RunningX64}
		; add 64bit driver file and register to normal registry
		Call WriteRegeDit64
	${EndIf}

SectionEnd

;--------------------------------
;Descriptions

	;Language strings
	LangString DESC_SecODBC ${LANG_ENGLISH} "ODBC Driver files, include 32bit and 64bit driver"

	;Assign language strings to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${SecODBC} $(DESC_SecODBC)
	!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\cubrid_odbc32.dll"
	Delete "$INSTDIR\cubrid_odbc64.dll"
	Delete "$INSTDIR\uninstall.exe"
	Delete "$INSTDIR\README.txt"
	RMDir "$INSTDIR"

	DeleteRegKey /ifempty HKLM "SOFTWARE\cubrid-odbc"
	DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\cubrid-odbc"
	
  SetRegView 32
	DeleteRegKey	HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver"
	DeleteRegValue	HKLM "SOFTWARE\ODBC\ODBCINST.INI\ODBC Drivers" "CUBRID Driver"

	${If} ${RunningX64}
		SetRegView 64
		DeleteRegKey	HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver"
		DeleteRegValue	HKLM "SOFTWARE\ODBC\ODBCINST.INI\ODBC Drivers" "CUBRID Driver"
		SetRegView 32
	${EndIf}

SectionEnd


Function WriteRegeDit32
    SetRegView 32
    File "cubrid_odbc32.dll"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "Driver" "$INSTDIR\cubrid_odbc32.dll"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "Setup" "$INSTDIR\cubrid_odbc32.dll"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "APILevel" "2"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "DriverODBCVer" "10.0.0(32)"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "ConnectFunctions" "YYN"
		
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "CreateDSN" "CUBRID Driver 32"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "SQLLevel" "0"
		WriteRegDWORD HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "UsageCoun" "00000001"
	  WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\ODBC Drivers" "CUBRID Driver" "Installed"
FunctionEnd

Function WriteRegeDit64
    SetRegView 64
		File "cubrid_odbc64.dll"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "Driver" "$INSTDIR\cubrid_odbc64.dll"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "Setup" "$INSTDIR\cubrid_odbc64.dll"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "Setup" "$INSTDIR\cubrid_odbc64.dll"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "APILevel" "2"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "DriverODBCVer" "10.0.0(64)"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "ConnectFunctions" "YYN"

		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "CreateDSN" "CUBRID Driver 64"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "SQLLevel" "0"
		WriteRegDWORD HKLM "SOFTWARE\ODBC\ODBCINST.INI\CUBRID Driver" "UsageCoun" "00000001"
		WriteRegStr HKLM "SOFTWARE\ODBC\ODBCINST.INI\ODBC Drivers" "CUBRID Driver" "Installed"
		SetRegView 32
FunctionEnd

Function CheckVCRedist
    Push $R9
    ClearErrors

    System::Call "kernel32::GetCurrentProcess() i .s"
    System::Call "kernel32::IsWow64Process(i s, *i .r0)"
    StrCmp $0 '0' Win32 Win64
        Win32:
        SetRegView 32
	GOTO EndBitCheck
        Win64:
        SetRegView 64 

EndBitCheck:

    ReadRegStr $R9 HKLM "SOFTWARE\Classes\Installer\Dependencies\Microsoft.VS.VC_RuntimeAdditionalVSU_x86,v14" "Version"
    SetRegView Default
    IfErrors 0 VSRedistInstalled
    StrCpy $R9 "No" 
    VSRedistInstalled:
        Exch $R9
FunctionEnd
