@echo off
setlocal

set count=%~1
set base_filename=%~2
set delta=%~3

set mouse_tracker_t="MouseTrackerT.exe"

call :GetExecutionTime %mouse_tracker_t%

endlocal
exit /b

:GetExecutionTime
set start_time=%time%

%1 "points" %count% "%base_filename%_query.crsdat" %delta%

set end_time=%time%

call :ParseTime "%start_time%" start_h start_m start_s start_cs
set /a start_total= %start_h% * 3600000 + %start_m% * 60000 + %start_s% * 1000 + %start_cs% * 10

call :ParseTime "%end_time%" end_h end_m end_s end_cs
set /a end_total= %end_h% * 3600000 + %end_m% * 60000 + %end_s% * 1000 + %end_cs% * 10

if %end_total% lss %start_total% (
    set /a diff= 86400000 - start_total + end_total
) else (
    set /a diff= end_total - start_total
)

echo Real duration: %diff%
exit /b

:ParseTime
set "time_str=%~1"
set "time_str=%time_str::=%"
set "time_str=%time_str:.=%"
set "time_str=%time_str: =0%"

set %2=%time_str:~0,2%
set %3=%time_str:~2,2%
set %4=%time_str:~4,2%
set %5=%time_str:~7,2%
set /a %2=%2
set /a %3=%3
set /a %4=%4
set /a %5=%5
exit /b