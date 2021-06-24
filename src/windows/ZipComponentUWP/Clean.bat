::
:: Deletes all intermediate and debug build files
::
@ECHO OFF
SET PROJECTPATH=%~dp0
SET DELFILES=DEL /S /Q /F %PROJECTPATH%

GOTO :StartClean


:: %1 Folder in which to delete recursively all specified file types
:CleanIntFiles
%DELFILES%%1\*.exp >NUL 2>&1
%DELFILES%%1\*.ilk >NUL 2>&1
%DELFILES%%1\*.iobj >NUL 2>&1
%DELFILES%%1\*.ipdb >NUL 2>&1
%DELFILES%%1\*.lib >NUL 2>&1
%DELFILES%%1\*.pdb >NUL 2>&1
%DELFILES%%1\*.pri >NUL 2>&1
EXIT /B 0

:: %1 Folder in which to delete all sub folders
:: %2 Optional pattern to match. Uses * if not specified
:DelSubFolders
SET PATTERN=%2
IF "%PATTERN%" =="" SET PATTERN=*
FOR /d %%D IN (%PROJECTPATH%%1\%PATTERN%) DO @IF EXIST "%%D" RMDIR /S /Q "%%D"
EXIT /B 0


:StartClean
CALL :CleanIntFiles Bin
CALL :DelSubFolders Bin *_Debug
CALL :DelSubFolders Bin ZipTest_*
CALL :DelSubFolders Build
CALL :CleanIntFiles Libraries\EmptyDotNetComponentUWP\Bin
CALL :DelSubFolders Libraries\EmptyDotNetComponentUWP\Bin *_Debug
CALL :DelSubFolders Libraries\EmptyDotNetComponentUWP\Build
