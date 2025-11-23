$cflags = (pkg-config --cflags gtkmm-4.0) -split ' ' | Where-Object { $_ -notmatch '-m(fpmath|sse)' }
$libs = (pkg-config --libs gtkmm-4.0) -split ' '

$arguments = @("-std=c++17") + $cflags + @("settingUp.cpp") + $libs + @("-o", "settingUp.exe")

& C:\msys64\ucrt64\bin\g++.exe $arguments