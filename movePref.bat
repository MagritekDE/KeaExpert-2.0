@ECHO OFF
ECHO ------------------------------------
ECHO Move Prospa preferences to backup
ECHO ------------------------------------
ECHO.
IF NOT EXIST "%USERPROFILE%\\Application Data\\Prospa\\Preferences V3.4" GOTO TRYWIN7
   CD "%USERPROFILE%\\Application Data\\Prospa"
   MD OldPref
   MOVE "Preferences V3.4" "OldPref\\Date(%date:/=-%)Time(%time::=-%))"
   ECHO Prospa preferences moved to OldPref
   GOTO EXIT
:TRYWIN7
IF NOT EXIST "%USERPROFILE%\\AppData\\Roaming\\Prospa\\Preferences V3.4" GOTO NOPREFDIR
   CD "%USERPROFILE%\\AppData\\Roaming\\Prospa"
   MD OldPref
   MOVE "Preferences V3.4" "OldPref\\Date(%date:/=-%)Time(%time::=-%))"
   ECHO Prospa preferences moved to OldPref
   GOTO EXIT
:NOPREFDIR
   ECHO Prospa Preferences does not exist
:EXIT
ECHO.
PAUSE