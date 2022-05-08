@echo off

rem working directory
cd %~dp0

cd build

del /S/F/Q .

cmake -D EVENT__DISABLE_OPENSSL=ON -D EVENT__LIBRARY_TYPE=STATIC -A WIN32 ..

cmake --build . --config Release

xcopy /S/Y include\* ..\include\win32\
copy /Y lib\Release\*.lib ..\lib\win32\

cd ..
