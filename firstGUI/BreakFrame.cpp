#include "BreakFrame.h"

BreakFrame::BreakFrame(const wxString& title,
    const wxString& message,
    const wxString& footer,
    int seconds,
    std::function<void()> onClosed)
    : wxFrame(
        nullptr,
        wxID_ANY,
        title,
        wxDefaultPosition,
        wxDefaultSize,
        wxSTAY_ON_TOP | wxBORDER_NONE
    ),
    m_timer(this),
    m_timeLeft(seconds),
    m_showMinutes(seconds >= 60),
    m_onClosed(std::move(onClosed))
{
    SetIcon(wxIcon("app_icon", wxBITMAP_TYPE_ICO_RESOURCE));

    wxSize frameSize(360, 560);

    SetClientSize(frameSize);
    SetMinSize(frameSize);
    SetMaxSize(frameSize);
    Centre();

    // Whole window background
    wxPanel* rootPanel = new wxPanel(this, wxID_ANY);
    rootPanel->SetBackgroundColour(wxColour(245, 248, 252));

    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(rootPanel, 1, wxEXPAND);
    SetSizer(frameSizer);

    // Main card
    wxPanel* cardPanel = new wxPanel(rootPanel, wxID_ANY);
    cardPanel->SetBackgroundColour(wxColour(255, 255, 255));

    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(cardPanel, 1, wxEXPAND | wxALL, 12);
    rootPanel->SetSizer(rootSizer);

    wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);
    cardSizer->AddSpacer(38);

    // Top image, embedded in the exe (see app.rc) so it loads no matter
    // which folder the app is started from.
    wxBitmap breakBitmap = wxBITMAP_PNG(eye_break);

    if (breakBitmap.IsOk())
    {
        wxImage breakImage = breakBitmap.ConvertToImage().Scale(270, 120, wxIMAGE_QUALITY_HIGH);

        wxStaticBitmap* imageBitmap = new wxStaticBitmap(
            cardPanel,
            wxID_ANY,
            wxBitmap(breakImage)
        );

        cardSizer->Add(imageBitmap, 0, wxALIGN_CENTER | wxBOTTOM, 25);
    }

    // Title
    wxStaticText* titleLabel = new wxStaticText(cardPanel, wxID_ANY, title);

    wxFont titleFont = titleLabel->GetFont();
    titleFont.SetPointSize(17);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleLabel->SetFont(titleFont);
    titleLabel->SetForegroundColour(wxColour(31, 78, 121));

    // Subtitle
    wxStaticText* subtitleLabel = new wxStaticText(
        cardPanel,
        wxID_ANY,
        message,
        wxDefaultPosition,
        wxDefaultSize,
        wxALIGN_CENTER
    );

    wxFont subtitleFont = subtitleLabel->GetFont();
    subtitleFont.SetPointSize(9);
    subtitleLabel->SetFont(subtitleFont);
    subtitleLabel->SetForegroundColour(wxColour(80, 90, 105));

    // Countdown container
    wxPanel* countdownPanel = new wxPanel(
        cardPanel,
        wxID_ANY,
        wxDefaultPosition,
        wxSize(120, 120)
    );

    countdownPanel->SetBackgroundColour(wxColour(245, 248, 252));

    wxBoxSizer* countdownSizer = new wxBoxSizer(wxVERTICAL);

    m_countdownLabel = new wxStaticText(countdownPanel, wxID_ANY, FormatTimeLeft());

    wxFont countdownFont = m_countdownLabel->GetFont();
    countdownFont.SetPointSize(22);
    countdownFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_countdownLabel->SetFont(countdownFont);
    m_countdownLabel->SetForegroundColour(wxColour(20, 45, 70));

    wxStaticText* unitLabel = new wxStaticText(
        countdownPanel,
        wxID_ANY,
        m_showMinutes ? "MINUTES" : "SECONDS"
    );

    wxFont unitFont = unitLabel->GetFont();
    unitFont.SetPointSize(7);
    unitFont.SetWeight(wxFONTWEIGHT_BOLD);
    unitLabel->SetFont(unitFont);
    unitLabel->SetForegroundColour(wxColour(90, 100, 115));

    countdownSizer->AddStretchSpacer(1);
    countdownSizer->Add(m_countdownLabel, 0, wxALIGN_CENTER);
    countdownSizer->Add(unitLabel, 0, wxALIGN_CENTER);
    countdownSizer->AddStretchSpacer(1);

    countdownPanel->SetSizer(countdownSizer);

    // One Close button only
    wxButton* closeBtn = new wxButton(
        cardPanel,
        wxID_ANY,
        "Close",
        wxDefaultPosition,
        wxSize(270, 38)
    );

    closeBtn->SetBackgroundColour(wxColour(41, 104, 145));
    closeBtn->SetForegroundColour(wxColour(255, 255, 255));

    closeBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        Close();
        });

    // Footer
    wxStaticText* footerLabel = new wxStaticText(cardPanel, wxID_ANY, footer);

    wxFont footerFont = footerLabel->GetFont();
    footerFont.SetPointSize(8);
    footerLabel->SetFont(footerFont);
    footerLabel->SetForegroundColour(wxColour(80, 90, 105));

    // Layout
    cardSizer->Add(titleLabel, 0, wxALIGN_CENTER | wxBOTTOM, 8);
    cardSizer->Add(subtitleLabel, 0, wxALIGN_CENTER | wxBOTTOM, 35);
    cardSizer->Add(countdownPanel, 0, wxALIGN_CENTER | wxBOTTOM, 38);
    cardSizer->Add(closeBtn, 0, wxALIGN_CENTER | wxBOTTOM, 35);
    cardSizer->Add(footerLabel, 0, wxALIGN_CENTER);

    cardPanel->SetSizer(cardSizer);

    Layout();

    Bind(wxEVT_TIMER, &BreakFrame::OnTimer, this);
    Bind(wxEVT_CLOSE_WINDOW, &BreakFrame::OnClose, this);
    m_timer.Start(1000);
}

wxString BreakFrame::FormatTimeLeft() const
{
    if (m_showMinutes)
    {
        return wxString::Format("%d:%02d", m_timeLeft / 60, m_timeLeft % 60);
    }

    return wxString::Format("%02d", m_timeLeft);
}

void BreakFrame::OnTimer(wxTimerEvent&)
{
    m_timeLeft--;

    m_countdownLabel->SetLabel(FormatTimeLeft());

    if (m_timeLeft <= 0)
    {
        Close();
    }
}

// Runs when the user clicks Close or the countdown finishes. Not called when
// MainFrame destroys the window directly (e.g. going to sleep), which is what
// we want: that case shouldn't restart the work timer.
void BreakFrame::OnClose(wxCloseEvent& event)
{
    m_timer.Stop();

    // Clear it first so a second Close() can't run it twice.
    std::function<void()> onClosed = std::move(m_onClosed);
    m_onClosed = nullptr;

    if (onClosed)
    {
        onClosed();
    }

    // Let the default handler destroy the window.
    event.Skip();
}
