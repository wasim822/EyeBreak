#include "EyeTimerFrame.h"
#include "resource1.h"
#include <wx/msw/private.h>
EyeTimerFrame::EyeTimerFrame(const wxString& title, int seconds)
    : wxFrame(
        nullptr,
        wxID_ANY,
        title,
        wxDefaultPosition,
        wxDefaultSize,
        wxSTAY_ON_TOP | wxBORDER_NONE
    ),
    m_timer(this),
    m_timeLeft(seconds)
{
    wxSize disp = wxGetDisplaySize();

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

    // Top image
    wxImage::AddHandler(new wxPNGHandler());

    wxImage breakImage("eye-break.png", wxBITMAP_TYPE_PNG);

    if (!breakImage.IsOk())
    {
        wxMessageBox("Could not load break.png", "Image Error");
    }
    else
    {
        breakImage = breakImage.Scale(270, 120, wxIMAGE_QUALITY_HIGH);
    }

    wxBitmap breakBitmap(breakImage);

    wxStaticBitmap* imageBitmap = new wxStaticBitmap(
        cardPanel,
        wxID_ANY,
        breakBitmap,
        wxDefaultPosition,
        wxSize(270, 120)
    );

    // Title
    wxStaticText* titleLabel = new wxStaticText(
        cardPanel,
        wxID_ANY,
        "Time for an Eye Break"
    );

    wxFont titleFont = titleLabel->GetFont();
    titleFont.SetPointSize(17);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleLabel->SetFont(titleFont);
    titleLabel->SetForegroundColour(wxColour(31, 78, 121));

    // Subtitle
    wxStaticText* subtitleLabel = new wxStaticText(
        cardPanel,
        wxID_ANY,
        "Every 20 minutes, look at something 20\nfeet away for 20 seconds.",
        wxDefaultPosition,
        wxDefaultSize,
        wxALIGN_CENTER
    );

    wxFont subtitleFont = subtitleLabel->GetFont();
    subtitleFont.SetPointSize(9);
    subtitleLabel->SetFont(subtitleFont);
    subtitleLabel->SetForegroundColour(wxColour(80, 90, 105));

    // Countdown container
    wxPanel* countdownPanel = new wxPanel(cardPanel, wxID_ANY, wxDefaultPosition, wxSize(105, 105));
    countdownPanel->SetBackgroundColour(wxColour(245, 248, 252));

    wxBoxSizer* countdownSizer = new wxBoxSizer(wxVERTICAL);

    m_countdownLabel = new wxStaticText(
        countdownPanel,
        wxID_ANY,
        wxString::Format("%d", m_timeLeft)
    );

    wxFont countdownFont = m_countdownLabel->GetFont();
    countdownFont.SetPointSize(20);
    countdownFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_countdownLabel->SetFont(countdownFont);
    m_countdownLabel->SetForegroundColour(wxColour(20, 45, 70));

    wxStaticText* secondsLabel = new wxStaticText(
        countdownPanel,
        wxID_ANY,
        "SECONDS"
    );

    wxFont secondsFont = secondsLabel->GetFont();
    secondsFont.SetPointSize(7);
    secondsFont.SetWeight(wxFONTWEIGHT_BOLD);
    secondsLabel->SetFont(secondsFont);
    secondsLabel->SetForegroundColour(wxColour(90, 100, 115));

    countdownSizer->AddStretchSpacer(1);
    countdownSizer->Add(m_countdownLabel, 0, wxALIGN_CENTER);
    countdownSizer->Add(secondsLabel, 0, wxALIGN_CENTER);
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
    wxStaticText* footerLabel = new wxStaticText(
        cardPanel,
        wxID_ANY,
        "Reducing digital eye strain"
    );

    wxFont footerFont = footerLabel->GetFont();
    footerFont.SetPointSize(8);
    footerLabel->SetFont(footerFont);
    footerLabel->SetForegroundColour(wxColour(80, 90, 105));

    // Layout
    cardSizer->AddSpacer(38);
    cardSizer->Add(imageBitmap, 0, wxALIGN_CENTER | wxBOTTOM, 25);
    cardSizer->Add(titleLabel, 0, wxALIGN_CENTER | wxBOTTOM, 8);
    cardSizer->Add(subtitleLabel, 0, wxALIGN_CENTER | wxBOTTOM, 35);
    cardSizer->Add(countdownPanel, 0, wxALIGN_CENTER | wxBOTTOM, 38);
    cardSizer->Add(closeBtn, 0, wxALIGN_CENTER | wxBOTTOM, 35);
    cardSizer->Add(footerLabel, 0, wxALIGN_CENTER);

    cardPanel->SetSizer(cardSizer);

    Layout();
    Refresh();

    Bind(wxEVT_TIMER, &EyeTimerFrame::OnTimer, this);
    m_timer.Start(1000);
}


void EyeTimerFrame::OnTimer(wxTimerEvent& event)
{
    m_timeLeft--;

    m_countdownLabel->SetLabel(
        wxString::Format("%02d", m_timeLeft)
    );

    if (m_timeLeft <= 0)
    {
        m_timer.Stop();
        Close();
    }
}