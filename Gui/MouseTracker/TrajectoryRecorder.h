#ifndef __MOUSE_TRACKER_IMGUI_TRAJECTORYRECORDER__
#define __MOUSE_TRACKER_IMGUI_TRAJECTORYRECORDER__

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")
#define NOMINMAX

#include <profileapi.h>
#include <string>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <chrono>
#include <timeapi.h>

namespace Mt
{
    class TrajectoryRecorder
    {
        public:
            std::vector<POINT> StartReadMouseTrajectoryRoutineTscCpuWait(int delay, int delta = 1)
            {
                std::vector<POINT> points;

                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

                TIMECAPS tc;
                timeGetDevCaps(&tc, sizeof(TIMECAPS));

                UINT targetResolution = (std::min)((std::max)(tc.wPeriodMin, 1u), tc.wPeriodMax);
                timeBeginPeriod(targetResolution);

                LARGE_INTEGER frequency;
                QueryPerformanceFrequency(&frequency);

                const double ticksPerMs = static_cast<double>(frequency.QuadPart) / 1000.0;
                const long long targetTickCount = static_cast<long long>(ticksPerMs * delta);

                LARGE_INTEGER startTime;
                QueryPerformanceCounter(&startTime);

                POINT point;
                GetCursorPos(&point);
                points.push_back(point);

                LARGE_INTEGER now;
                POINT initialPoint = point;

                while (true)
                {
                    QueryPerformanceCounter(&startTime);

                    GetCursorPos(&point);
                    
                    if (initialPoint.x != point.x || initialPoint.y != point.y)
                    {
                        points.push_back(point);
                        break;
                    }

                    QueryPerformanceCounter(&now);
                    while ((now.QuadPart - startTime.QuadPart) < targetTickCount)
                        QueryPerformanceCounter(&now);
                }

                POINT lastPoint = point;
                LARGE_INTEGER lastMoveTime = startTime;

                while (true)
                {
                    QueryPerformanceCounter(&startTime);

                    GetCursorPos(&point);
                    points.push_back(point);

                    if (point.x == lastPoint.x && point.y == lastPoint.y)
                    {
                        long long elapsedTicks = startTime.QuadPart - lastMoveTime.QuadPart;
                        double elapsedMs = static_cast<double>(elapsedTicks) / ticksPerMs;

                        if (elapsedMs >= delay)
                            break;
                    }
                    else
                    {
                        lastPoint = point;
                        lastMoveTime = startTime;
                    }

                    QueryPerformanceCounter(&now);
                    while ((now.QuadPart - startTime.QuadPart) < targetTickCount)
                        QueryPerformanceCounter(&now);
                }

                timeEndPeriod(targetResolution);
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

                return points;
            }

            std::vector<POINT> StartReadCursorRoutineTscCpuWait(int count, int delta = 1)
            {
                std::vector<POINT> points;
                points.reserve(count);

                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

                TIMECAPS tc;
                timeGetDevCaps(&tc, sizeof(TIMECAPS));

                UINT targetResolution = (std::min)((std::max)(tc.wPeriodMin, 1u), tc.wPeriodMax);
                timeBeginPeriod(targetResolution);

                LARGE_INTEGER frequency;
                QueryPerformanceFrequency(&frequency);

                const double ticksPerMs = static_cast<double>(frequency.QuadPart) / 1000.0;
                const long long targetTickCount = static_cast<long long>(ticksPerMs * delta);

                // LARGE_INTEGER calibratingStart;
                // LARGE_INTEGER calibratingEnd;

                // QueryPerformanceCounter(&calibratingStart);
                // Sleep(1);
                // QueryPerformanceCounter(&calibratingEnd);

                // const long long sleepOverhead = calibratingEnd.QuadPart - calibratingStart.QuadPart - static_cast<long long>(ticksPerMs);

                for (int i = 0; i < count; i++)
                {
                    LARGE_INTEGER startTime;
                    QueryPerformanceCounter(&startTime);

                    POINT point;
                    GetCursorPos(&point);
                    points.push_back(point);

                    LARGE_INTEGER now;
                    QueryPerformanceCounter(&now);

                    while ((now.QuadPart - startTime.QuadPart) < targetTickCount)
                        QueryPerformanceCounter(&now);
                }

                timeEndPeriod(targetResolution);
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

                return points;
            }

            std::vector<POINT> StartReadCursorRoutineTimerWindowsEx(int count, int delta = 1)
            {
                std::vector<POINT> points;
                points.reserve(count);

                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

                TIMECAPS tc;
                timeGetDevCaps(&tc, sizeof(TIMECAPS));

                UINT timerResolution = (std::min)((std::max)(tc.wPeriodMin, 1u), tc.wPeriodMax);
                timeBeginPeriod(timerResolution);

                LARGE_INTEGER freq;
                QueryPerformanceFrequency(&freq);

                using clock = std::chrono::high_resolution_clock;
                LARGE_INTEGER dueTime;

                HANDLE hTimer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);

                auto recordStart = clock::now();
                POINT point;
                GetCursorPos(&point);
                points.push_back(point);
                dueTime.QuadPart = -static_cast<LONGLONG>(0);
                long long recordElapsed = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - recordStart).count();
                int a = 0;
                a++;
                a++;
                long long recordSleepTime = (std::max)((delta * 1000 - recordElapsed - a) * 10, 0LL);
                SetWaitableTimer(hTimer, &dueTime, 0, NULL, NULL, 0);
                WaitForSingleObject(hTimer, INFINITE);
                auto recordEnd = clock::now();
                long long dt = std::chrono::duration_cast<std::chrono::microseconds>(recordEnd - recordStart).count();

                for (int i = 0; i < count; i++)
                {
                    auto startTime = clock::now();

                    POINT point;
                    GetCursorPos(&point);
                    points.push_back(point);

                    long long elapsed = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - startTime).count();

                    if (delta * 1000 > elapsed)
                    {
                        //Timer dt 100ns
                        dueTime.QuadPart = -static_cast<LONGLONG>((delta * 1000 - elapsed - dt) * 10);

                        SetWaitableTimer(hTimer, &dueTime, 0, NULL, NULL, 0);

                        WaitForSingleObject(hTimer, INFINITE);
                    }
                }

                CloseHandle(hTimer);

                timeEndPeriod(timerResolution);

                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

                return points;
            }

            std::vector<POINT> StartReadMouseTrajectoryContinuousTscCpuWait(int endDelay, int delta = 1)
            {
                std::vector<POINT> points;

                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

                TIMECAPS tc;
                timeGetDevCaps(&tc, sizeof(TIMECAPS));

                UINT targetResolution = (std::min)((std::max)(tc.wPeriodMin, 1u), tc.wPeriodMax);
                timeBeginPeriod(targetResolution);

                LARGE_INTEGER frequency;
                QueryPerformanceFrequency(&frequency);

                const double ticksPerMs = static_cast<double>(frequency.QuadPart) / 1000.0;
                const long long targetTickCount = static_cast<long long>(ticksPerMs * delta);

                LARGE_INTEGER startTime;
                QueryPerformanceCounter(&startTime);

                POINT point;
                GetCursorPos(&point);
                points.push_back(point);

                POINT lastPoint = point;
                LARGE_INTEGER lastMoveTime = startTime;

                while (true)
                {
                    QueryPerformanceCounter(&startTime);

                    GetCursorPos(&point);
                    points.push_back(point);

                    if (point.x != lastPoint.x || point.y != lastPoint.y)
                    {
                        lastPoint = point;
                        lastMoveTime = startTime;
                    }
                    else
                    {
                        long long elapsedTicks = startTime.QuadPart - lastMoveTime.QuadPart;
                        double elapsedMs = static_cast<double>(elapsedTicks) / ticksPerMs;

                        if (elapsedMs >= endDelay)
                            break;
                    }

                    LARGE_INTEGER now;
                    QueryPerformanceCounter(&now);

                    while ((now.QuadPart - startTime.QuadPart) < targetTickCount)
                        QueryPerformanceCounter(&now);
                }

                timeEndPeriod(targetResolution);
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

                return points;
            }

            ~TrajectoryRecorder() = default;
    };
}

#endif
