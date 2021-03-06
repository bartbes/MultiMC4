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
#include <wx/listctrl.h>

#include "insticonlist.h"

class ChangeIconDialog : public wxDialog
{
public:
	ChangeIconDialog(wxWindow *parent, InstIconList *iconList);
	
	wxString GetSelectedIconKey() const;
	
protected:
	InstIconList *m_iconList;
	
	void OnItemActivated(wxListEvent &event);
	
	class InstIconListCtrl : public wxListCtrl
	{
	public:
		InstIconListCtrl(wxWindow *parent, InstIconList *iconList);
		
		void UpdateItems();
		
	protected:
		InstIconList *m_iconList;
	} *iconListCtrl;
	
	DECLARE_EVENT_TABLE()
};
