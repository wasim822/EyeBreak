#include "InfoFrame.h"
#include "MainFrame.h"
#include "Startup.h"

#include <memory>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

namespace
{
    // Colours used across the window
    const wxColour PAGE_BG(241, 245, 249);
    const wxColour CARD_BG(255, 255, 255);
    const wxColour SOFT_BG(246, 248, 251);
    const wxColour BORDER(226, 232, 240);
    const wxColour ACCENT(37, 99, 235);
    const wxColour ACCENT_HOVER(29, 78, 216);
    const wxColour TEXT_DARK(15, 23, 42);
    const wxColour TEXT_MUTED(100, 116, 139);
    const wxColour GOOD_BG(220, 252, 231);
    const wxColour GOOD_TEXT(21, 128, 61);
    const wxColour PAUSED_BG(254, 243, 199);
    const wxColour PAUSED_TEXT(180, 83, 9);
    const wxColour DANGER(185, 28, 28);
    const wxColour DANGER_HOVER_BG(254, 242, 242);

    // 1200 -> "20 minutes", 20 -> "20 seconds"
    wxString DescribeDuration(int seconds)
    {
        if (seconds >= 60 && seconds % 60 == 0)
        {
            int minutes = seconds / 60;
            return wxString::Format(minutes == 1 ? "%d minute" : "%d minutes", minutes);
        }

        return wxString::Format(seconds == 1 ? "%d second" : "%d seconds", seconds);
    }

    // 3600 -> "every hour", 1200 -> "every 20 minutes"
    wxString DescribeEvery(int seconds)
    {
        if (seconds >= 3600 && seconds % 3600 == 0)
        {
            int hours = seconds / 3600;
            return hours == 1 ? wxString("every hour") : wxString::Format("every %d hours", hours);
        }

        return "every " + DescribeDuration(seconds);
    }

    // 1200 -> "20 min", 20 -> "20 sec"
    wxString DescribeShort(int seconds)
    {
        if (seconds >= 60 && seconds % 60 == 0)
        {
            return wxString::Format("%d min", seconds / 60);
        }

        return wxString::Format("%d sec", seconds);
    }

    void StyleText(wxWindow* window, int pointSize, wxFontWeight weight, const wxColour& colour)
    {
        wxFont font = window->GetFont();
        font.SetPointSize(pointSize);
        font.SetWeight(weight);
        window->SetFont(font);
        window->SetForegroundColour(colour);
    }

    // Panel with rounded corners. It is drawn on demand instead of using an
    // image, so it costs no more memory than a plain panel. Controls placed
    // on it pick up its colour.
    class RoundedPanel : public wxPanel
    {
    public:
        RoundedPanel(wxWindow* parent, const wxColour& fill, int radius, const wxColour& border = wxNullColour)
            : wxPanel(parent, wxID_ANY),
            m_radius(radius),
            m_border(border)
        {
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            SetBackgroundColour(fill);

            Bind(wxEVT_PAINT, &RoundedPanel::OnPaint, this);
            Bind(wxEVT_SIZE, [this](wxSizeEvent& event) { Refresh(); event.Skip(); });
        }

    private:
        int m_radius;
        wxColour m_border;

        void OnPaint(wxPaintEvent&)
        {
            wxAutoBufferedPaintDC dc(this);
            dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
            dc.Clear();

            std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

            if (!gc)
            {
                return;
            }

            wxSize size = GetClientSize();
            gc->SetBrush(wxBrush(GetBackgroundColour()));
            gc->SetPen(m_border.IsOk() ? wxPen(m_border) : *wxTRANSPARENT_PEN);
            gc->DrawRoundedRectangle(0.5, 0.5, size.x - 1, size.y - 1, m_radius);
        }
    };

    // Borderless button that changes colour under the mouse.
    wxButton* MakeFlatButton(wxWindow* parent, const wxString& label, int height, int pointSize,
        const wxColour& bg, const wxColour& hoverBg, const wxColour& fg)
    {
        wxButton* button = new wxButton(parent, wxID_ANY, label, wxDefaultPosition, wxSize(-1, height), wxBORDER_NONE);
        button->SetBackgroundColour(bg);
        StyleText(button, pointSize, wxFONTWEIGHT_SEMIBOLD, fg);
        button->SetCursor(wxCursor(wxCURSOR_HAND));

        button->Bind(wxEVT_ENTER_WINDOW, [button, hoverBg](wxMouseEvent& event)
        {
            button->SetBackgroundColour(hoverBg);
            button->Refresh();
            event.Skip();
        });

        button->Bind(wxEVT_LEAVE_WINDOW, [button, bg](wxMouseEvent& event)
        {
            button->SetBackgroundColour(bg);
            button->Refresh();
            event.Skip();
        });

        return button;
    }

    // Small grey heading above a group, e.g. "SCHEDULE"
    wxStaticText* MakeSectionTitle(wxWindow* parent, const wxString& text)
    {
        wxStaticText* label = new wxStaticText(parent, wxID_ANY, text);
        StyleText(label, 8, wxFONTWEIGHT_SEMIBOLD, TEXT_MUTED);
        return label;
    }

    // Tile for one break type: caption, big value and a detail line.
    RoundedPanel* MakeTile(wxWindow* parent, const wxString& caption, wxStaticText*& value, wxStaticText*& detail)
    {
        RoundedPanel* tile = new RoundedPanel(parent, SOFT_BG, 10, BORDER);

        wxStaticText* captionLabel = new wxStaticText(tile, wxID_ANY, caption);
        StyleText(captionLabel, 8, wxFONTWEIGHT_SEMIBOLD, TEXT_MUTED);

        value = new wxStaticText(tile, wxID_ANY, "");
        StyleText(value, 16, wxFONTWEIGHT_BOLD, ACCENT);

        detail = new wxStaticText(tile, wxID_ANY, "");
        StyleText(detail, 8, wxFONTWEIGHT_NORMAL, TEXT_MUTED);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->AddSpacer(12);
        sizer->Add(captionLabel, 0, wxLEFT | wxRIGHT, 12);
        sizer->AddSpacer(2);
        sizer->Add(value, 0, wxLEFT | wxRIGHT, 12);
        sizer->Add(detail, 0, wxLEFT | wxRIGHT, 12);
        sizer->AddSpacer(12);
        tile->SetSizer(sizer);

        return tile;
    }
}

InfoFrame::InfoFrame(MainFrame* owner, const BreakSettings& settings)
    : wxFrame(
        nullptr,
        wxID_ANY,
        "Eye Break",
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)
    ),
    m_owner(owner),
    m_settings(settings)
{
    SetIcon(wxIcon("app_icon", wxBITMAP_TYPE_ICO_RESOURCE));

    // Space between the card edge and its contents
    const int padding = 20;

    // Whole window background
    wxPanel* rootPanel = new wxPanel(this, wxID_ANY);
    rootPanel->SetBackgroundColour(PAGE_BG);

    // Main card
    RoundedPanel* cardPanel = new RoundedPanel(rootPanel, CARD_BG, 14, BORDER);

    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(cardPanel, 1, wxEXPAND | wxALL, 14);
    rootPanel->SetSizer(rootSizer);

    // Title
    wxStaticText* titleLabel = new wxStaticText(cardPanel, wxID_ANY, "Eye Break");
    StyleText(titleLabel, 18, wxFONTWEIGHT_BOLD, TEXT_DARK);

    // Status pill, coloured and labelled by UpdateStatus()
    m_statusPill = new RoundedPanel(cardPanel, GOOD_BG, 11);

    m_statusLabel = new wxStaticText(m_statusPill, wxID_ANY, "");
    StyleText(m_statusLabel, 8, wxFONTWEIGHT_SEMIBOLD, GOOD_TEXT);

    wxBoxSizer* statusSizer = new wxBoxSizer(wxVERTICAL);
    statusSizer->AddSpacer(4);
    statusSizer->Add(m_statusLabel, 0, wxLEFT | wxRIGHT, 12);
    statusSizer->AddSpacer(4);
    m_statusPill->SetSizer(statusSizer);

    // Title on the left, status on the right
    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    headerSizer->Add(titleLabel, 0, wxALIGN_CENTER_VERTICAL);
    headerSizer->AddStretchSpacer();
    headerSizer->Add(m_statusPill, 0, wxALIGN_CENTER_VERTICAL);

    // About text
    wxStaticText* aboutLabel = new wxStaticText(
        cardPanel,
        wxID_ANY,
        "Every 20 minutes, look at something 20 feet away for 20 seconds."
    );
    StyleText(aboutLabel, 9, wxFONTWEIGHT_NORMAL, TEXT_MUTED);
    aboutLabel->Wrap(300);

    // Schedule tiles, filled in from the current settings
    wxBoxSizer* tilesSizer = new wxBoxSizer(wxHORIZONTAL);
    tilesSizer->Add(MakeTile(cardPanel, "EYE BREAK", m_eyeBreakValue, m_eyeBreakDetail), 1, wxEXPAND | wxRIGHT, 10);
    tilesSizer->Add(MakeTile(cardPanel, "LONG BREAK", m_longBreakValue, m_longBreakDetail), 1, wxEXPAND);
    UpdateScheduleText();

    // Settings group
    RoundedPanel* settingsPanel = new RoundedPanel(cardPanel, SOFT_BG, 10, BORDER);

    wxStaticText* longBreakLabel = new wxStaticText(settingsPanel, wxID_ANY, "Long break length");
    StyleText(longBreakLabel, 9, wxFONTWEIGHT_NORMAL, TEXT_DARK);

    m_longBreakSpin = new wxSpinCtrl(
        settingsPanel,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(64, -1),
        wxSP_ARROW_KEYS,
        BreakSettings::MIN_LONG_BREAK_MINUTES,
        BreakSettings::MAX_LONG_BREAK_MINUTES,
        m_settings.longBreakSeconds / 60);
    m_longBreakSpin->Bind(wxEVT_SPINCTRL, &InfoFrame::OnLongBreakChanged, this);

    wxStaticText* minutesLabel = new wxStaticText(settingsPanel, wxID_ANY, "min");
    StyleText(minutesLabel, 9, wxFONTWEIGHT_NORMAL, TEXT_MUTED);

    wxBoxSizer* longBreakSizer = new wxBoxSizer(wxHORIZONTAL);
    longBreakSizer->Add(longBreakLabel, 0, wxALIGN_CENTER_VERTICAL);
    longBreakSizer->AddStretchSpacer();
    longBreakSizer->Add(m_longBreakSpin, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    longBreakSizer->Add(minutesLabel, 0, wxALIGN_CENTER_VERTICAL);

    // Thin line between the two settings
    wxPanel* divider = new wxPanel(settingsPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    divider->SetBackgroundColour(BORDER);

    m_startupCheck = new wxCheckBox(settingsPanel, wxID_ANY, "Start when I sign in to Windows");
    StyleText(m_startupCheck, 9, wxFONTWEIGHT_NORMAL, TEXT_DARK);
    m_startupCheck->SetValue(Startup::IsEnabled());
    m_startupCheck->Bind(wxEVT_CHECKBOX, &InfoFrame::OnStartupToggled, this);

    wxBoxSizer* settingsSizer = new wxBoxSizer(wxVERTICAL);
    settingsSizer->Add(longBreakSizer, 0, wxEXPAND | wxALL, 12);
    settingsSizer->Add(divider, 0, wxEXPAND | wxLEFT | wxRIGHT, 12);
    settingsSizer->Add(m_startupCheck, 0, wxEXPAND | wxALL, 12);
    settingsPanel->SetSizer(settingsSizer);

    // Primary action is pause / resume; Close sits beside it as the quiet option.
    m_pauseBtn = MakeFlatButton(cardPanel, "", 38, 10, ACCENT, ACCENT_HOVER, *wxWHITE);
    m_pauseBtn->Bind(wxEVT_BUTTON, &InfoFrame::OnPauseClicked, this);

    wxButton* closeBtn = MakeFlatButton(cardPanel, "Close", 38, 10, SOFT_BG, BORDER, TEXT_DARK);
    closeBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Close(); });

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(m_pauseBtn, 2, wxEXPAND | wxRIGHT, 8);
    buttonSizer->Add(closeBtn, 1, wxEXPAND);

    UpdateStatus();

    // Footer, below a divider so Stop is kept well away from Close
    wxPanel* footerDivider = new wxPanel(cardPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    footerDivider->SetBackgroundColour(BORDER);

    wxStaticText* footerLabel = new wxStaticText(cardPanel, wxID_ANY, "Closing keeps it running in the tray.");
    StyleText(footerLabel, 8, wxFONTWEIGHT_NORMAL, TEXT_MUTED);

    wxButton* stopBtn = MakeFlatButton(cardPanel, "Stop Eye Break", 26, 8, CARD_BG, DANGER_HOVER_BG, DANGER);
    stopBtn->Bind(wxEVT_BUTTON, &InfoFrame::OnStopClicked, this);

    wxBoxSizer* footerSizer = new wxBoxSizer(wxHORIZONTAL);
    footerSizer->Add(footerLabel, 0, wxALIGN_CENTER_VERTICAL);
    footerSizer->AddStretchSpacer();
    footerSizer->Add(stopBtn, 0, wxALIGN_CENTER_VERTICAL);

    // Layout. Every row gets the same side padding; gaps between rows are spacers.
    const int sides = wxLEFT | wxRIGHT;

    wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);
    cardSizer->AddSpacer(padding);
    cardSizer->Add(headerSizer, 0, wxEXPAND | sides, padding);
    cardSizer->AddSpacer(4);
    cardSizer->Add(aboutLabel, 0, sides, padding);
    cardSizer->AddSpacer(16);
    cardSizer->Add(MakeSectionTitle(cardPanel, "SCHEDULE"), 0, sides, padding);
    cardSizer->AddSpacer(6);
    cardSizer->Add(tilesSizer, 0, wxEXPAND | sides, padding);
    cardSizer->AddSpacer(16);
    cardSizer->Add(MakeSectionTitle(cardPanel, "SETTINGS"), 0, sides, padding);
    cardSizer->AddSpacer(6);
    cardSizer->Add(settingsPanel, 0, wxEXPAND | sides, padding);
    cardSizer->AddSpacer(18);
    cardSizer->Add(buttonSizer, 0, wxEXPAND | sides, padding);
    cardSizer->AddSpacer(16);
    cardSizer->Add(footerDivider, 0, wxEXPAND | sides, padding);
    cardSizer->AddSpacer(8);
    cardSizer->Add(footerSizer, 0, wxEXPAND | sides, padding);
    cardSizer->AddSpacer(12);

    cardPanel->SetSizer(cardSizer);

    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(rootPanel, 1, wxEXPAND);
    SetSizerAndFit(frameSizer);

    Centre();
}

void InfoFrame::AskAboutStartup()
{
    Startup::SetAsked();

    if (Startup::IsEnabled())
    {
        return;
    }

    int answer = wxMessageBox(
        "Would you like Eye Break to start automatically when you sign in to Windows?\n\n"
        "You can change this at any time in this window.",
        "Start with Windows?",
        wxYES_NO | wxICON_QUESTION,
        this);

    if (answer == wxYES)
    {
        if (!Startup::SetEnabled(true))
        {
            wxMessageBox("Eye Break couldn't be added to startup.", "Eye Break", wxOK | wxICON_WARNING, this);
        }

        m_startupCheck->SetValue(Startup::IsEnabled());
    }
}

void InfoFrame::UpdateScheduleText()
{
    m_eyeBreakValue->SetLabel(DescribeShort(m_settings.eyeBreakSeconds));
    m_eyeBreakDetail->SetLabel(DescribeEvery(m_settings.workSeconds));

    // Every Nth break is the long one, so it comes every N work periods.
    m_longBreakValue->SetLabel(DescribeShort(m_settings.longBreakSeconds));
    m_longBreakDetail->SetLabel(DescribeEvery(m_settings.workSeconds * m_settings.breaksPerLongBreak));
}

void InfoFrame::UpdateStatus()
{
    bool paused = m_owner->IsPaused();

    m_statusPill->SetBackgroundColour(paused ? PAUSED_BG : GOOD_BG);
    m_statusLabel->SetForegroundColour(paused ? PAUSED_TEXT : GOOD_TEXT);
    m_statusLabel->SetLabel(paused ? wxString(L"●  Paused") : wxString(L"●  Running"));
    m_pauseBtn->SetLabel(paused ? "Resume breaks" : "Pause breaks");

    m_statusPill->Refresh();
    m_statusPill->GetParent()->Layout();
}

void InfoFrame::OnPauseClicked(wxCommandEvent&)
{
    if (m_owner->IsPaused())
    {
        m_owner->Resume();
    }
    else
    {
        m_owner->Pause();
    }

    UpdateStatus();
}

void InfoFrame::OnLongBreakChanged(wxSpinEvent&)
{
    m_settings.longBreakSeconds = m_longBreakSpin->GetValue() * 60;

    // Saved right away, so it's kept the next time the app starts.
    m_owner->SetLongBreakSeconds(m_settings.longBreakSeconds);

    UpdateScheduleText();
    m_longBreakValue->GetParent()->Layout();
}

void InfoFrame::OnStartupToggled(wxCommandEvent&)
{
    Startup::SetAsked();

    if (!Startup::SetEnabled(m_startupCheck->GetValue()))
    {
        wxMessageBox("Eye Break couldn't change the startup setting.", "Eye Break", wxOK | wxICON_WARNING, this);
    }

    m_startupCheck->SetValue(Startup::IsEnabled());
}

void InfoFrame::OnStopClicked(wxCommandEvent&)
{
    int answer = wxMessageBox(
        "Stop Eye Break?\n\nYou won't get any break reminders until you open the app again.",
        "Stop Eye Break",
        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,
        this);

    if (answer == wxYES)
    {
        // Closes this window too.
        m_owner->Quit();
    }
}
