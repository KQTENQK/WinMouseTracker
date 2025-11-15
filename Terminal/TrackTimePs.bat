@echo off
setlocal

set count=%~1
set base_filename=%~2
set delta=%~3

set mouse_tracker_t="MouseTrackerT.exe"

powershell -command "Write-Host $PWD; Set-Location '%CD%'; $start = [System.Diagnostics.Stopwatch]::StartNew(); & $PWD/%mouse_tracker_t% 'points' %count% '%base_filename%_query.crsdat' %delta%; $elapsed = $start.ElapsedMilliseconds; Write-Host 'Real duration:' $elapsed 'ms'"

endlocal