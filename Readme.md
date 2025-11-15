# Windows Mouse Tracker

Applications to track mouse movement: record a set of points over dt or capture trajectory over dt up to no-movement delay.

## /Terminal/

Terminal app to track mouse.

```main.cpp``` - Main application with all capture methods

```Compile.bat``` - Batch script to compile (from developer command prompt)

```TrackTimeCmd.bat``` & ```TrackTimePs.bat``` - Execution time measurement scripts


## /ShowChartUtility/

Python-based trajectory visualization tool.

Can be used to process trajectory and save them without showing (in silent mode, for a set of trajectories).

```show_2d_points.py``` - 2D trajectory plotting with matplotlib

```build.py``` - PyInstaller build script for standalone executable


## /Gui/

Graphical app to capture trajectories: record a set of points over dt or capture trajectory up to no-movement delay.

Recording might be performed in minimized mode by hotkey trigger.

Depends on ImGui 1.92.4.

Trajectory data format is:

```
x;y
x;y
...
```

<img src="/GitAssets/GuiView.png">
