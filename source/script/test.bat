@echo off
:LOOP_AGAIN
idr.exe -p COM32 -c ""sam;info;quit;"
timeout 5
goto :LOOP_AGAIN