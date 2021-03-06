/*
    Copyright 2012 Andrew Okin

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


#include "consolewindow.h"

#include <wx/gbsizer.h>
#include <wx/sstream.h>
#include <gui/mainwindow.h>

#include "multimc.h"

InstConsoleWindow::InstConsoleWindow(Instance *inst, wxWindow* mainWin)
	: wxFrame(NULL, -1, _("MultiMC Console"), wxDefaultPosition, wxSize(620, 250)),
	  instListener(inst, this)
{
	instListenerStarted = false;
	m_mainWin = mainWin;
	m_inst = inst;
	inst->SetEvtHandler(this);
	
	wxPanel *mainPanel = new wxPanel(this, -1);
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainPanel->SetSizer(mainSizer);
	
	wxString launchCmdMessage = wxString::Format(_("Instance started with command: %s\n"), 
		inst->GetLastLaunchCommand().c_str());
	
	consoleTextCtrl = new wxTextCtrl(mainPanel, -1, launchCmdMessage, 
									 wxDefaultPosition, wxSize(200, 100), 
									 wxTE_MULTILINE | wxTE_READONLY);
	mainSizer->Add(consoleTextCtrl, wxSizerFlags(1).Expand().Border(wxALL, 8));
	
	wxBoxSizer *btnBox = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(btnBox, wxSizerFlags(0).Align(wxALIGN_BOTTOM | wxALIGN_RIGHT).
				   Border(wxBOTTOM | wxRIGHT, 8));
	
	closeButton = new wxButton(mainPanel, wxID_CLOSE, _("&Close"));
	
	// disable close button and the X button provided by the window manager
	AllowClose(false);
	btnBox->Add(closeButton, wxSizerFlags(0).Align(wxALIGN_RIGHT));
	
	
	// Create the task bar icon.
	trayIcon = new ConsoleIcon(this);
	trayIcon->SetIcon(wxGetApp().GetAppIcon());
	
	inst->GetInstProcess()->SetNextHandler(this);
	
	instListener.Create();
	
	CenterOnScreen();
}

InstConsoleWindow::~InstConsoleWindow()
{
	instListener.Delete();
}

void InstConsoleWindow::AppendMessage(const wxString& msg)
{
	(*consoleTextCtrl) << msg << _("\n");
}

void InstConsoleWindow::OnInstExit(wxProcessEvent& event)
{
	AppendMessage(wxString::Format(_("Instance exited with code %i."), 
		event.GetExitCode()));
	
	AllowClose();
	if (event.GetExitCode() != 0)
	{
		AppendMessage(_("Minecraft has crashed!"));
		Show();
	}
	else if (settings.GetAutoCloseConsole())
	{
		Close();
	}
	else
	{
		Show();
	}
}

void InstConsoleWindow::AllowClose(bool allow)
{
	if(allow)
	{
		EnableCloseButton(true);
		m_closeAllowed = true;
		closeButton->Enable();
	}
	else
	{
		EnableCloseButton(false);
		m_closeAllowed = false;
		closeButton->Enable(false);
	}
}

void InstConsoleWindow::Close()
{
	wxFrame::Close();
	if (trayIcon->IsIconInstalled())
		trayIcon->RemoveIcon();
	m_mainWin->Show();
}

bool InstConsoleWindow::Show(bool show)
{
	bool retval = wxFrame::Show(show);
	if (!instListenerStarted)
		instListener.Run();
	return retval;
}

bool InstConsoleWindow::Start()
{
	return Show(settings.GetShowConsole());
}


void InstConsoleWindow::OnCloseClicked(wxCommandEvent& event)
{
	Close();
}

void InstConsoleWindow::OnWindowClosed(wxCloseEvent& event)
{
	if (event.CanVeto() && !m_closeAllowed)
	{
		event.Veto();
	}
	else
	{
		if (trayIcon->IsIconInstalled())
			trayIcon->RemoveIcon();
		m_mainWin->Show();
		Destroy();
	}
}

InstConsoleWindow::InstConsoleListener::InstConsoleListener(Instance* inst, InstConsoleWindow *console)
	: wxThread(wxTHREAD_JOINABLE)
{
	m_inst = inst;
	m_console = console;
	instProc = inst->GetInstProcess();
}

void* InstConsoleWindow::InstConsoleListener::Entry()
{
	if (!instProc->IsRedirected())
	{
		printf("Output not redirected!\n");
		m_console->AppendMessage(_("Output not redirected!\n"));
		return NULL;
	}
	
	int instPid = instProc->GetPid();
	
	wxInputStream *consoleStream = instProc->GetInputStream();
	wxString outputBuffer;
	
	const size_t bufSize = 1024;
	char *buffer = new char[bufSize];
	
	size_t readSize = 0;
	while (m_inst->IsRunning() && !TestDestroy() && wxProcess::Exists(instPid))
	{
		if (TestDestroy())
			break;
		
		// Read from input
		wxString temp;
		wxStringOutputStream tempStream(&temp);
		
		consoleStream->Read(buffer, bufSize);
		readSize = consoleStream->LastRead();
		
		tempStream.Write(buffer, readSize);
		outputBuffer.Append(temp);
		
		// Pass lines to the console
		size_t newlinePos;
		while ((newlinePos = outputBuffer.First('\n')) != wxString::npos)
		{
			wxString line = outputBuffer.Left(newlinePos);
			if (line.EndsWith(_("\n")) || line.EndsWith(_("\r")))
				line = line.Left(line.size() - 1);
			if (line.EndsWith(_("\r\n")))
				line = line.Left(line.size() - 2);
			outputBuffer = outputBuffer.Mid(newlinePos + 1);
			
			InstOutputEvent event(m_inst, line);
			m_console->AddPendingEvent(event);
		}
	}

	return NULL;
}

void InstConsoleWindow::OnInstOutput(InstOutputEvent& event)
{
	AppendMessage(event.m_output);
}

InstConsoleWindow::ConsoleIcon::ConsoleIcon(InstConsoleWindow *console)
{
	m_console = console;
}

Instance *InstConsoleWindow::GetInstance()
{
	return m_inst;
}

void InstConsoleWindow::StopListening()
{
	instListener.Pause();
}

wxMenu *InstConsoleWindow::ConsoleIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu();
	menu->AppendCheckItem(ID_SHOW_CONSOLE, _("Show Console"), _("Shows or hides the console."))->
		Check(m_console->IsShown());
	menu->Append(ID_KILL_MC, _("Kill Minecraft"), _("Kills Minecraft's process."));
	
	return menu;
}

void InstConsoleWindow::ConsoleIcon::OnShowConsole(wxCommandEvent &event)
{
	m_console->Show(event.IsChecked());
}

void InstConsoleWindow::ConsoleIcon::OnKillMC(wxCommandEvent &event)
{
	if (wxMessageBox(_("Killing Minecraft may damage saves. You should only do this if the game is frozen."),
		_("Are you sure?"), wxOK | wxCANCEL | wxCENTER) == wxOK)
	{
		wxProcess *instProc = m_console->GetInstance()->GetInstProcess();
		
		if (instProc->GetPid() == 0)
			return;
		
		int pid = instProc->GetPid();
		
		m_console->StopListening();
		wxKillError error = wxProcess::Kill(pid, wxSIGTERM);
		if (error != wxKILL_OK)
		{
			wxString errorName;
			switch (error)
			{
			case wxKILL_ACCESS_DENIED:
				errorName = _("wxKILL_ACCESS_DENIED");
				break;
				
			case wxKILL_BAD_SIGNAL:
				errorName = _("wxKILL_BAD_SIGNAL");
				break;
				
			case wxKILL_ERROR:
				errorName = _("wxKILL_ERROR");
				break;
				
			case wxKILL_NO_PROCESS:
				errorName = _("wxKILL_NO_PROCESS");
				break;
				
			default:
				errorName = _("Unknown error.");
			}
			
			wxLogError(_("Error %i (%s) when killing process %i!"), error, errorName.c_str(), 
				m_console->GetInstance()->GetInstProcess()->GetPid());
		}
		else
		{
			m_console->AppendMessage(wxString::Format(_("Killed Minecraft (pid: %i)"), pid));
			wxProcessEvent fakeEvent(0, pid, -1);
			m_console->OnInstExit(fakeEvent);
		}
	}
}


BEGIN_EVENT_TABLE(InstConsoleWindow, wxFrame)
	EVT_END_PROCESS(-1, InstConsoleWindow::OnInstExit)
	EVT_BUTTON(wxID_CLOSE, InstConsoleWindow::OnCloseClicked)
	EVT_CLOSE( InstConsoleWindow::OnWindowClosed )
	EVT_INST_OUTPUT(InstConsoleWindow::OnInstOutput)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(InstConsoleWindow::ConsoleIcon, wxTaskBarIcon)
	EVT_MENU(ID_SHOW_CONSOLE, InstConsoleWindow::ConsoleIcon::OnShowConsole)
	EVT_MENU(ID_KILL_MC, InstConsoleWindow::ConsoleIcon::OnKillMC)
END_EVENT_TABLE()