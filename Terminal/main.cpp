#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")
#define NOMINMAX

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <windows.h>
#include <fstream>
#include <profileapi.h>
#include <algorithm>

std::vector<POINT> StartReadMouseTrajectoryRoutineTscCpuWait(int delay, int delta = 1)
{
    std::vector<POINT> points;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));

    UINT targetResolution = std::min(std::max(tc.wPeriodMin, 1u), tc.wPeriodMax);
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
        points.push_back(point);

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

    UINT targetResolution = std::min(std::max(tc.wPeriodMin, 1u), tc.wPeriodMax);
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

    UINT timerResolution = std::min(std::max(tc.wPeriodMin, 1u), tc.wPeriodMax);
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
    long long recordSleepTime = std::max((delta * 1000 - recordElapsed - a) * 10, 0LL);
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

int ReadCursorPoints(int argc, char* argv[])
{
    int count = 10000;
    int delta = 1;
    std::string filename = std::string("./cursor_data");
    
    if (argc != 4 && argc != 1)
    {
        std::cout << "Usage: " << std::string(argv[0]) << " <count> <filename> <delta>." << std::endl;

        return -1;
    }

    if (argc == 1)
        std::cout << "Usage: " << std::string(argv[0]) << " <count> <filename> <delta>." << std::endl;

    if (argc == 4)
    {
        count = std::stoi(argv[1]);
        filename = std::string(argv[2]);
        delta = std::stoi(argv[3]);
    }
    
    std::cout << "Record count: " << count << std::endl;
    std::cout << "Filename: " << filename << std::endl;

    std::vector<POINT> cursorPoints = StartReadCursorRoutineTscCpuWait(count, delta);

    std::ofstream file;

    file.open(filename);

    if (!file.is_open())
    {
        std::cout << "Unable to open file " << filename << "." << std::endl;

        return -2;
    }

    for (const auto& point : cursorPoints)
        file << point.x << ";" << point.y << "\n";

    file.close();

    return 0;
}

int ReadMouseTrajectory(int argc, char* argv[])
{
    int delay = 500;
    int delta = 1;
    std::string filename = std::string("./cursor_data");
    
    if (argc != 4 && argc != 1)
    {
        std::cout << "Usage: " << std::string(argv[0]) << " <delay> <filename> <delta>." << std::endl;

        return -1;
    }

    if (argc == 1)
        std::cout << "Usage: " << std::string(argv[0]) << " <delay> <filename> <delta>." << std::endl;

    if (argc == 4)
    {
        delay = std::stoi(argv[1]);
        filename = std::string(argv[2]);
        delta = std::stoi(argv[3]);
    }
    
    std::vector<POINT> cursorPoints = StartReadMouseTrajectoryRoutineTscCpuWait(delay, delta);

    std::ofstream file;

    std::cout << "Filename: " << filename << std::endl;

    file.open(filename);

    if (!file.is_open())
    {
        std::cout << "Unable to open file " << filename << "." << std::endl;

        return -2;
    }

    for (const auto& point : cursorPoints)
        file << point.x << ";" << point.y << "\n";

    file.close();

    return 0;
}

void PrintUsage(const std::string& programName)
{
    std::cout << "Usage: " << programName << " <mode> [parameters]" << std::endl;
    std::cout << "Modes:" << std::endl;
    std::cout << "  points     - Record fixed number of cursor points" << std::endl;
    std::cout << "               Usage: " << programName << " points <count> <filename> <delta>" << std::endl;
    std::cout << "  trajectory - Record mouse trajectory until idle" << std::endl;
    std::cout << "               Usage: " << programName << " trajectory <delay> <filename> <delta>" << std::endl;
    std::cout << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  count    - Number of points to record (for points mode)" << std::endl;
    std::cout << "  delay    - Idle time in ms to stop recording (for trajectory mode)" << std::endl;
    std::cout << "  filename - Output filename" << std::endl;
    std::cout << "  delta    - Time between samples in ms (default: 1)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " points 1000 points.txt 1" << std::endl;
    std::cout << "  " << programName << " trajectory 500 trajectory.txt 1" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        PrintUsage(argv[0]);

        return 0;
    }

    std::string mode = argv[1];

    if (mode == "points")
    {
        int newArgc = argc - 1;
        char** newArgv = new char*[newArgc + 1];
        
        newArgv[0] = argv[0];
        
        for (int i = 1; i < newArgc; i++)
            newArgv[i] = argv[i + 1];
        
        int result = ReadCursorPoints(newArgc, newArgv);
        delete[] newArgv;

        return result;
    }

    else if (mode == "trajectory")
    {
        int newArgc = argc - 1;
        char** newArgv = new char*[newArgc + 1];
        
        newArgv[0] = argv[0];
        
        for (int i = 1; i < newArgc; i++)
            newArgv[i] = argv[i + 1];
        
        int result = ReadMouseTrajectory(newArgc, newArgv);
        delete[] newArgv;

        return result;
    }

    else
    {
        std::cout << "Unknown mode: " << mode << std::endl;
        PrintUsage(argv[0]);

        return -1;
    }
}
