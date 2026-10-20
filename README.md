# Eye Break

A small Windows background app that helps you follow the **20-20-20 rule** and take regular breaks from the screen.

> Every 20 minutes, look at something 20 feet away for 20 seconds.

Eye Break runs quietly in the system tray. Every 20 minutes it pops up a small window reminding you to rest your eyes, with a countdown. After every third eye break you get a longer 5-minute break to stand up, stretch and drink some water.

## Features

- **Eye breaks**: a 20-second reminder every 20 minutes.
- **Long breaks**: every 4th break is a 5-minute stand-up break.
- **Info window**: opening the app (from the desktop icon, Start menu, or by double-clicking the tray icon) shows a small window explaining the app, with a **Start with Windows** checkbox and a **Stop Eye Break** button.
- **Runs in the background**: closing the info window does not stop the reminders. The app keeps running in the system tray; right-click the tray icon to open it or exit.
- **Starts with Windows**: the first time you open the app, it asks whether to start automatically when you sign in. When it starts this way, it runs silently without showing the info window.
- **Light on resources**: the app only wakes up every 30 seconds (or exactly when a break is due), so it uses almost no CPU.
- **Sleep aware**: if the PC sleeps, hibernates or is away for more than a minute, the time away counts as a break and the cycle starts over.
- **Single instance**: opening it again while it's running shows the info window of the running copy instead of starting a second one.
- **Self-contained**: the icon and break image are embedded in `EyeBreak.exe`, so the exe works from any folder.

## How the cycle works

```
work 20 min -> eye break 20 s -> work 20 min -> eye break 20 s
            -> work 20 min -> eye break 20 s -> work 20 min -> long break 5 min -> (repeat)
```

Closing a break window early with the **Close** button starts the next 20-minute work period right away.

## Building

### Requirements

- Windows 10 or 11
- Visual Studio 2026 (or newer) with the **Desktop development with C++** workload
- [wxWidgets](https://www.wxwidgets.org/) 3.3, built as static libraries (`vc_lib` / `vc_x64_lib`)

### Steps

1. Download and build wxWidgets (open `build\msw\wx_vc*.sln` in the wxWidgets folder and build the Debug and Release configurations for x64).
2. Set the `WXWIN` environment variable to your wxWidgets folder, for example:
   ```
   setx WXWIN C:\path\to\wxWidgets-3.3.2
   ```
   Restart Visual Studio after setting it.
3. Open `firstGUI.slnx` in Visual Studio.
4. Choose **Release | x64** and build. The app is written to `x64\Release\EyeBreak.exe`.

## Starting with Windows

The first time you open Eye Break, it asks whether it should start automatically when you sign in to Windows. You can turn this on or off at any time with the **Start Eye Break when I sign in to Windows** checkbox in the info window.

How it works:

- The setting is stored in the current user's `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` registry key, so no admin rights are needed. It also shows up in Windows **Settings > Apps > Startup**.
- Windows launches the app as `EyeBreak.exe --startup`. The `--startup` flag tells the app to run silently in the tray without opening the info window.
- If you move `EyeBreak.exe` to a different folder, open it once from the new location and the startup entry updates itself.
- Opening the app while it is already running brings up the running copy's info window instead of starting a second copy.

To put Eye Break on your desktop, right-click `EyeBreak.exe` and choose **Show more options > Send to > Desktop (create shortcut)**.

## Changing the break times

All timing values live in [`firstGUI/BreakSettings.h`](firstGUI/BreakSettings.h):

| Setting              | Default  | Meaning                                         |
| -------------------- | -------- | ----------------------------------------------- |
| `workSeconds`        | 1200     | Time between breaks (20 minutes)                |
| `eyeBreakSeconds`    | 20       | Length of an eye break                          |
| `longBreakSeconds`   | 300      | Length of a long break (5 minutes)              |
| `breaksPerLongBreak` | 4        | Every Nth break is a long break                 |
| `sleepResetSeconds`  | 60       | Time away after which the cycle restarts        |

For testing, set `workSeconds` to something small like `5` so breaks appear quickly. These values are hard-coded for now; the plan is to load them from a settings file or settings window later.

## Project layout

```
firstGUI/
  App.cpp / App.h                Entry point: single-instance check, --startup flag, creates MainFrame
  MainFrame.cpp / MainFrame.h    Hidden window that runs the break schedule and tray icon
  BreakFrame.cpp / BreakFrame.h  The popup break window (used for both break types)
  InfoFrame.cpp / InfoFrame.h    Info window: about text, startup checkbox, Stop button
  Startup.cpp / Startup.h        Turns "start with Windows" on and off (registry)
  BreakSettings.h                All timing values
  app.rc                         Embeds eyeIcon.ico and eye-break.png into the exe
  eyeIcon.ico, eye-break.png     App icon and break image
```

## License

No license has been chosen yet.
