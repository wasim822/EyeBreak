#pragma once

#include <wx/wx.h>
#include <wx/msw/registry.h>

// All timing values in one place. Most are hard-coded for now; the long
// break length can be changed by the user and is saved in the registry.
struct BreakSettings
{
    // How long to work before a break (20 minutes).
    // For testing, set this to something small like 5.
    int workSeconds = 20 * 60;
    //int workSeconds = 5;

    // Length of the short 20-20-20 eye break.
    int eyeBreakSeconds = 20;

    // Length of the long stand-up break.
    int longBreakSeconds = 5 * 60;

    // Every Nth break is a long break (2 eye breaks, then a long one), so
    // with 20 minute work periods the long break comes every hour.
    int breaksPerLongBreak = 3;

    // If the timer misses ticks by more than this, the PC was asleep or
    // locked, so start the cycle over.
    int sleepResetSeconds = 60;

    // Range the user can pick for the long break, in minutes.
    static const int MIN_LONG_BREAK_MINUTES = 1;
    static const int MAX_LONG_BREAK_MINUTES = 60;

    // Reads the user's saved values, keeping the defaults for anything missing.
    void Load()
    {
        wxLogNull noLog;

        wxRegKey key(wxRegKey::HKCU, "Software\\EyeBreak");
        long minutes = 0;

        if (key.Exists() && key.HasValue("LongBreakMinutes") && key.QueryValue("LongBreakMinutes", &minutes)
            && minutes >= MIN_LONG_BREAK_MINUTES && minutes <= MAX_LONG_BREAK_MINUTES)
        {
            longBreakSeconds = (int)minutes * 60;
        }
    }

    void Save() const
    {
        wxLogNull noLog;

        wxRegKey key(wxRegKey::HKCU, "Software\\EyeBreak");

        if (key.Create())
        {
            key.SetValue("LongBreakMinutes", (long)(longBreakSeconds / 60));
        }
    }
};
