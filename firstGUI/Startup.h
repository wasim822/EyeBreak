#pragma once

// Command-line flag added to the startup entry, so the app knows it was
// launched by Windows at sign-in (and shouldn't show the info window).
#define STARTUP_FLAG "--startup"

// Controls whether Eye Break starts when the user signs in to Windows.
// Uses the current user's Run key, so no admin rights are needed.
namespace Startup
{
    bool IsEnabled();
    bool SetEnabled(bool enable);

    // If startup is on but the exe was moved, point the entry at the new path.
    void RefreshPath();

    // Whether the user has already been asked to turn startup on.
    bool WasAsked();
    void SetAsked();
}
