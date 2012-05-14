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
#include <wx/dnd.h>
#include <instance.h>

class ModEditDialog : public wxDialog
{
public:
	ModEditDialog(wxWindow *parent, Instance *inst);
	
protected:
	class ModListCtrl : public wxListCtrl
	{
	public:
		ModListCtrl(wxWindow *parent, int id, Instance *inst);
		
		virtual wxString OnGetItemText(long int item, long int column) const;
		
		virtual void UpdateItems();
		
		virtual void SetInsertMark(const int index);
		
		void DrawInsertMark(int index);
		
	protected:
		Instance *m_inst;
		
		int m_insMarkIndex;
	} *jarModList;
	
	wxListCtrl *mlModList;
	
	void LoadJarMods();
	void LoadMLMods();
	
	void UpdateColSizes();
	
	virtual bool Show(bool show = true);
	
	Instance *m_inst;
	
	void OnDeleteJarMod();
	void OnJarListKeyDown(wxListEvent &event);
	
	class ModsDropTarget : public wxFileDropTarget
	{
	public:
		ModsDropTarget(ModListCtrl *owner, Instance *inst);
		
		virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);
		
		virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
		virtual void OnLeave();
		
		ModListCtrl *m_owner;
		Instance *m_inst;
	};
	
	DECLARE_EVENT_TABLE()
};

enum
{
	ID_JAR_MOD_LIST,
	ID_ML_MOD_LIST,
};