@echo off
:rem ������ĿĿ¼λ��
set CoolViewDir=..\CoolView
set SipWrapperDir=..\CvSipWrapper\build
set FilterRegDir=..\filters\_build\Win32
set RtpWareDir=..\filters\RtpWare
set IntelRegDir=..\filters\hwcodec\_build\Win32
set MsdxDir=..\msdx

:rem �ű�ִ�й��̿�ʼ
echo ��ȷ���汾���Ѿ����£�
:rem ѡ�����ģʽ
call :SELECT_MODE
echo Set compile mode to %CompileMode%
echo.
:rem ѡ��Ҫ���µ����
call :SELECT_COMPONENT
@echo Update complete!
pause
GOTO :EOF
:rem �ű�ִ�н���


:rem ѡ�����ģʽ
:SELECT_MODE
    choice /c rd /m "Update Debug or Release version"
    :rem ���뵹������
    if errorlevel 255 goto :EOF
    if errorlevel 2 (
        set CompileMode=Debug
        goto :EOF
    )
    if errorlevel 1 (
        set CompileMode=Release
        goto :EOF
    )
    GOTO :EOF


:rem ѡ��Ҫ���µ����
:SELECT_COMPONENT
    echo Choose a component to update:
    echo     1.CoolView(NetworkMeasurer not included)
    echo     2.SipWrapper
    echo     3.MSDX
    echo     4.RtpWare(including rtp related filters and NetworkMeasurer)
    echo     5.Filters(including Intel HW codec, but not rtp filters)
    echo     6.All components
    choice /c 123456 /m "Choice"
    if errorlevel 255 goto :EOF
    if errorlevel 6 (
        call :UPDATE_COOLVIEW
        call :UPDATE_SIPWARPPER
        call :UPDATE_MSDX
        call :UPDATE_RTPWARE
        call :UPDATE_FILTERS
        goto :EOF
    )
    if errorlevel 5 (
        call :UPDATE_FILTERS
        goto :EOF
    )
    if errorlevel 4 (
        call :UPDATE_RTPWARE
        goto :EOF
    )
    if errorlevel 3 (
        call :UPDATE_MSDX
        goto :EOF
    )
    if errorlevel 2 (
        call :UPDATE_SIPWARPPER
        goto :EOF
    )
    if errorlevel 1 (
        call :UPDATE_COOLVIEW
        goto :EOF
    )
    GOTO :EOF


:rem ����CoolView���ļ�
:UPDATE_COOLVIEW
    echo Update project in %CoolViewDir%...
    if not exist %CompileMode%\CoolView (
        mkdir %CompileMode%\CoolView
    )
    for /F "tokens=1,2 delims=." %%i in (FileList\CoolView.txt) do (
        if "%CompileMode%"=="Debug" (
            if "%%i"=="cwebpage" (
                call :COPY_FILE %CoolViewDir%\%CompileMode%\%%id.%%j %CompileMode%\CoolView\%%id.%%j
            ) else (
                call :COPY_FILE %CoolViewDir%\%CompileMode%\%%i.%%j %CompileMode%\CoolView\%%i.%%j
                call :COPY_FILE %CoolViewDir%\%CompileMode%\%%i.pdb %CompileMode%\CoolView\%%i.pdb
            )
        ) else (
            call :COPY_FILE %CoolViewDir%\%CompileMode%\%%i.%%j %CompileMode%\CoolView\%%i.%%j
        )
    )
    @echo.
    GOTO :EOF


:rem ����CvSipWrapper���ļ�
:UPDATE_SIPWARPPER
    call :UPDATE_PROJECT %SipWrapperDir% CvSipWrapper FileList\CvSipWrapper.txt
    GOTO :EOF


:rem ����msdx���ļ�
:UPDATE_MSDX
    call :UPDATE_PROJECT %MsdxDir% msdx FileList\Msdx.txt
    GOTO :EOF


:rem ����Ҫע���Filter���ļ�
:UPDATE_FILTERS
    call :UPDATE_PROJECT %FilterRegDir% filter FileList\FilterReg.txt
    call :UPDATE_PROJECT %IntelRegDir% filter FileList\IntelFilter.txt
    GOTO :EOF


:rem ����ortp���ļ�
:UPDATE_RTPWARE
    call :UPDATE_PROJECT %RtpWareDir% ortp FileList\RtpWare.txt
    call :UPDATE_PROJECT %RtpWareDir% filter FileList\RtpFilter.txt
    :rem ����NetworkMeasurer���ļ�
    call :COPY_FILE %RtpWareDir%\%CompileMode%\NetworkMeasurer.exe %CompileMode%\CoolView\NetworkMeasurer.exe
    if "%CompileMode%"=="Debug" (
        call :COPY_FILE %RtpWareDir%\%CompileMode%\NetworkMeasurer.pdb %CompileMode%\CoolView\NetworkMeasurer.pdb
    )
    echo.
    GOTO :EOF


:rem ͨ�õ���Ŀ���º���
:rem ����1����ĿĿ¼
:rem ����2��Ŀ��Ŀ¼
:rem ����3���б��ļ�
:UPDATE_PROJECT
    echo Update project in %1
    if not exist %CompileMode%\%2 (
        mkdir %CompileMode%\%2
    )
    for /F "tokens=1,2 delims=." %%i in (%3) do (
        call :COPY_FILE %1\%CompileMode%\%%i.%%j %CompileMode%\%2\%%i.%%j
        if "%CompileMode%"=="Debug" (
            if exist %1\%CompileMode%\%%i.pdb (
                call :COPY_FILE %1\%CompileMode%\%%i.pdb %CompileMode%\%2\%%i.pdb
            ) else (
                @echo No pdb file for %1\%CompileMode%\%%i.%%j
            )
        )
    )
    @echo.
    @echo.
    GOTO :EOF


:rem �����ļ���������
:rem ����1��Դ�ļ�
:rem ����2��Ŀ��λ�ã����ļ�����
:COPY_FILE
    setlocal ENABLEDELAYEDEXPANSION
    for /f %%i in ("%1") do (
        set t=%%~ti
        :rem ��ʱ��תΪ�ɱȽ��ַ�����ʱ��Ӧ����2013/11/27 14:55�����ĸ�ʽ
        set ts=!t:~0,4!!t:~5,2!!t:~8,2!!t:~11,2!!t:~14,2!
        for /f %%j in ("%2") do (
            set t=%%~tj
            set td=!t:~0,4!!t:~5,2!!t:~8,2!!t:~11,2!!t:~14,2!
            if /I "!ts!" LSS "!td!" (
                echo     Source:%1
                echo     Dest:%2
                choice /m "Source is old than the dest. Replace anyway"
                if errorlevel 2 goto :EOF
            )
        )
    )
    echo copy %1 to %2
    copy /Y %1 %2
    GOTO :EOF
