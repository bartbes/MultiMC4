// 
//  Copyright 2012 Andrew Okin
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#pragma once
#include <wx/wx.h>

#include <map>

#include "task.h"
#include "logintask.h"
#include "checkupdatetask.h"

#include "insticonlist.h"

#include "settingsdialog.h"

//const wxString tbarIconPrefix = _T("resources/toolbar/");

class wxListbookEvent;
class wxListbook;

class MainWindow : public wxFrame
{
public:
	MainWindow(void);
	~MainWindow(void);

	void OnStartup();

	// Toolbar
	void OnAddInstClicked(wxCommandEvent& event);
	void OnViewFolderClicked(wxCommandEvent& event);
	void OnRefreshClicked(wxCommandEvent& event);

	void OnSettingsClicked(wxCommandEvent& event);
	void OnCheckUpdateClicked(wxCommandEvent& event);

	void OnHelpClicked(wxCommandEvent& event);
	void OnAboutClicked(wxCommandEvent& event);


	// Instance menu
	void OnPlayClicked(wxCommandEvent& event);
	void OnInstActivated(wxListEvent& event);
	
	void OnRenameClicked(wxCommandEvent& event);
	void OnCopyInstClicked(wxCommandEvent &event);
	void OnChangeIconClicked(wxCommandEvent& event);
	void OnNotesClicked(wxCommandEvent& event);
	void OnCancelEditNotesClicked(wxCommandEvent& event);

	void OnManageSavesClicked(wxCommandEvent& event);
	void OnEditModsClicked(wxCommandEvent& event);
	void OnRebuildJarClicked(wxCommandEvent& event);
	void OnViewInstFolderClicked(wxCommandEvent& event);
	
	void OnDeleteClicked(wxCommandEvent& event);
	
	
	// Task Events
	void OnTaskStart(TaskEvent &event);
	void OnTaskEnd(TaskEvent &event);
	void OnTaskProgress(TaskProgressEvent &event);
	void OnTaskStatus(TaskStatusEvent &event);
	void OnTaskError(TaskErrorEvent &event);
	
	void OnLoginComplete(LoginCompleteEvent &event);
	
	void OnCheckUpdateComplete(CheckUpdateEvent &event);
	
	// Other events
	void OnInstMenuOpened(wxListEvent& event);
	void OnWindowClosed(wxCloseEvent& event);
	
	
	// Other methods
	void StartTask(Task &task);
	bool StartModalTask(Task &task, bool forceModal = true);
	
	void ShowLoginDlg(wxString errorMsg);
	
	void LoadInstanceList(wxFileName instDir = settings.GetInstDir());
	
	
	DECLARE_EVENT_TABLE()

protected:
	wxMenu *instMenu;
	
	bool modalTaskRunning;
	
	bool canLaunch;
	
	bool closeOnTaskEnd;

	InstIconList instIcons;
	void LoadInstIconList(wxString customIconDirName = _T("icons"));

	void AddInstance(Instance *inst);

	Instance* GetLinkedInst(int id);

	Instance* GetSelectedInst();

	std::map<int, Instance*> instItems;
	
	bool GetNewInstName(wxString *instName, wxString *instDirName, const wxString title = _("Create new instance"));
	
	GUIMode GetGUIMode() const;
	
	// Basic GUI (a simple list control with a context menu)
	void InitBasicGUI(wxBoxSizer *mainSz);
	
	wxListCtrl *instListCtrl;
	
	
	// Advanced GUI
	void InitAdvancedGUI(wxBoxSizer *mainSz);
	
	void OnPageChanged(wxListbookEvent &event);

	void UpdateInstPanel();
	void UpdateInstNameLabel(Instance *inst);
	
	wxListbook *instListbook;
	wxPanel *instPanel;
	wxGridBagSizer *instSz;
	
	wxButton *btnPlay;
	wxButton *btnRename;
	wxButton *btnChangeIcon;
	wxButton *btnCopyInst;
	wxButton *btnEditMods;
	wxButton *btnRebuildJar;
	wxButton *btnViewFolder;
	
	wxTextCtrl *instNotesEditor;
	wxButton *editNotesBtn;
	wxButton *cancelEditNotesBtn;
	
	wxBoxSizer *instNameSz;
	wxStaticText *instNameLabel;
	wxTextCtrl *instNameEditor;
	
	void StartEditNotes();
	void FinishEditNotes();
	void CancelEditNotes();
	bool editingNotes;
	
	void StartRename();
	void FinishRename();
	void CancelRename();
	void OnRenameEnterPressed(wxCommandEvent &event);
	bool renamingInst;
	
	void EnableInstActions(bool enabled = true);
	void DisableInstActions();
	bool instActionsEnabled;
	
private:
	void NotImplemented();
};

enum
{
	// Toolbar
	ID_AddInst = 1,
	ID_ViewFolder,
	ID_ModsFolder,
	ID_Refresh,

	ID_Settings,
	ID_CheckUpdate,

	ID_Help,
	ID_About,

	// Instance menu
	ID_Play,

	ID_Rename,
	ID_CopyInst,
	ID_ChangeIcon,
	ID_EditNotes,
	ID_Cancel_EditNotes,

	ID_ManageSaves,
	ID_EditMods,
	ID_RebuildJar,
	ID_ViewInstFolder,

	ID_DeleteInst,

	// Other
	ID_InstListCtrl,
	
	ID_InstNameEditor,
};

