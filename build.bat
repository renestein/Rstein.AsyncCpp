@echo off
set ERROR_NO_VS=-1;
set ERROR_BAD_OPTIONS=-2;
set LIB_WIN_CL_STD20_AWAIT=lib_cl_win_std20_await
set LIB_WIN_CL_LEGACY_AWAIT=lib_cl_win_legacy_await
set LIB_CLANG_WIN_LEGACY_AWAIT=lib_clang_win_legacy_await
set CPP_HEADERS_FOLDER=bin\libs\includes\asynccpp
set RSTEIN_ASYNC_LIB_FOLDER=RStein.AsyncCpp

set TARGET=%1
rem internal commands
if "%TARGET%"=="INTERNAL_GENERATE_HEADERS" goto internal_command_regenerate_headers

goto public_commands
rem end internal commands

rem internal commands handlers
:internal_command_regenerate_headers
cd %~dp0
if exist "%CPP_HEADERS_FOLDER%" rmdir /q /s "%CPP_HEADERS_FOLDER%"
mkdir "%CPP_HEADERS_FOLDER%"
xcopy "%RSTEIN_ASYNC_LIB_FOLDER%\*.h" "%CPP_HEADERS_FOLDER%" /s /y
exit /b 0
rem end internal commands handlers

:public_commands
if "%TARGET%"=="" set TARGET=%LIB_WIN_CL_STD20_AWAIT%
if not "%TARGET%"=="%LIB_WIN_CL_STD20_AWAIT%" (
if not "%TARGET%"=="%LIB_WIN_CL_LEGACY_AWAIT%" (
if not "%TARGET%"=="%LIB_CLANG_WIN_LEGACY_AWAIT%" goto showusage))

set VS_LOCATION=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\
echo %VS_LOCATION%
if not exist "%VS_LOCATION%vcvars32.bat" (
  echo ERROR. Unable to locate Visual Studio. Exiting.
  cd %~dp0
  exit /b ERROR_NO_VS
)



echo Preparing build environment. Calling vcvars32.bat.
call "%VS_LOCATION%vcvars32.bat"
if %ERRORLEVEL% NEQ 0 goto build_failed

cd %~dp0


if %TARGET%==%LIB_WIN_CL_STD20_AWAIT% goto setup_win_cl_std20_await
if %TARGET%==%LIB_WIN_CL_LEGACY_AWAIT% goto setup_win_cl_legacy_await
if %TARGET%==%LIB_CLANG_WIN_LEGACY_AWAIT% goto setup_win_clang_legacy_await

:setup_win_cl_std20_await
set DEBUG_CONFIGURATION=Debug
set RELEASE_CONFIGURATION=Release
goto build_solution

:setup_win_cl_legacy_await
set DEBUG_CONFIGURATION=Debug_VSAWAIT
set RELEASE_CONFIGURATION=Release_VSAWAIT
goto build_solution

:setup_win_clang_legacy_await
set DEBUG_CONFIGURATION=Debug_ClangWin
set RELEASE_CONFIGURATION=Release_CLangWin
goto build_solution

:build_solution
:delete headers
cd %~dp0
echo Deleting old header files.

if exist "%CPP_HEADERS_FOLDER%" rmdir /q /s "%CPP_HEADERS_FOLDER%"

:clean_projects
echo Cleaning previous builds.
Msbuild "RStein.AsyncCppLib.sln"  /p:configuration=%RELEASE_CONFIGURATION% /p:platform=x64 /t:clean
if %ERRORLEVEL% NEQ 0 goto build_failed
Msbuild "RStein.AsyncCppLib.sln"  /p:configuration=%DEBUG_CONFIGURATION% /p:platform=x64 /t:clean
if %ERRORLEVEL% NEQ 0 goto build_failed
Msbuild "RStein.AsyncCppLib.sln"  /p:configuration=%RELEASE_CONFIGURATION% /p:platform=x86 /t:clean
if %ERRORLEVEL% NEQ 0 goto build_failed
Msbuild "RStein.AsyncCppLib.sln"  /p:configuration=%DEBUG_CONFIGURATION% /p:platform=x86 /t:clean
if %ERRORLEVEL% NEQ 0 goto build_failed

:build_projects
echo Building: Platform=x64 Configuration=%RELEASE_CONFIGURATION%.
msbuild  "RStein.AsyncCppLib.sln" /p:configuration=%RELEASE_CONFIGURATION% /p:platform=x64 /t:build
if %ERRORLEVEL% NEQ 0 goto build_failed

echo Building: Platform=x64 Configuration=%DEBUG_CONFIGURATION%.
msbuild  "RStein.AsyncCppLib.sln" /p:configuration=%DEBUG_CONFIGURATION% /p:platform=x64 /t:build

echo Building: Platform=x86 Configuration=%RELEASE_CONFIGURATION%.
msbuild  "RStein.AsyncCppLib.sln" /p:configuration=%RELEASE_CONFIGURATION% /p:platform=x86 /t:build
if %ERRORLEVEL% NEQ 0 goto build_failed

echo Building: Platform=x86 Configuration=%DEBUG_CONFIGURATION%.
msbuild  "RStein.AsyncCppLib.sln" /p:configuration=%DEBUG_CONFIGURATION% /p:platform=x86 /t:build
if %ERRORLEVEL% NEQ 0 goto build_failed

:create_headers
cd %~dp0
echo Generating header files.
echo.
echo.
if not exist "%CPP_HEADERS_FOLDER%" mkdir "%CPP_HEADERS_FOLDER%"
xcopy "%RSTEIN_ASYNC_LIB_FOLDER%\*.h" "%CPP_HEADERS_FOLDER%" /s /y
echo.
echo.
echo Build succeeded.
goto end:

:build_failed
echo.
echo.
echo Build failed.
cd %~dp0
exit /b %ERRORLEVEL%

:showusage
echo Usage:
echo ------
echo build.bat [target]
echo target: "%LIB_WIN_CL_STD20_AWAIT%|%LIB_WIN_CL_LEGACY_AWAIT%|%LIB_CLANG_WIN_LEGACY_AWAIT%. Default is %LIB_WIN_CL_STD20_AWAIT%."
cd %~dp0
exit /b %ERROR_BAD_OPTIONS%

:end
 exit /b 0