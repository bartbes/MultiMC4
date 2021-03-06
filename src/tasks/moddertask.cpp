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

#include "moddertask.h"
#include <fsutils.h>
#include <wx/wfstream.h>
#include <wx/fs_mem.h>
#include <apputils.h>

#if MSVC
#include <memory>
#else
#include <auto_ptr.h>
#endif

ModderTask::ModderTask(Instance* inst)
	: Task()
{
	m_inst = inst;
	step = 0;
}

void ModderTask::TaskStart()
{
	// Get the mod list
	const ModList *modList = m_inst->GetModList();
	
	wxFileName mcBin = m_inst->GetBinDir();
	wxFileName mcJar = m_inst->GetMCJar();
	wxFileName mcBackup = m_inst->GetMCBackup();
	
	SetStatus(_("Installing mods - backing up minecraft.jar..."));
	if (!mcBackup.FileExists() && !wxCopyFile(mcJar.GetFullPath(), mcBackup.GetFullPath()))
	{
		OnFail(_("Failed to back up minecraft.jar"));
		return;
	}
	
	if (mcJar.FileExists() && !wxRemoveFile(mcJar.GetFullPath()))
	{
		OnFail(_("Failed to delete old minecraft.jar"));
		return;
	}
	
	
	if (TestDestroy())
		return;
	TaskStep(); // STEP 1
	SetStatus(_("Installing mods - Opening minecraft.jar"));

	wxFFileOutputStream jarStream(mcJar.GetFullPath());
	wxZipOutputStream zipOut(jarStream);

	{
		wxFFileInputStream inStream(mcBackup.GetFullPath());
		wxZipInputStream zipIn(inStream);
		
		std::auto_ptr<wxZipEntry> entry;
		while (entry.reset(zipIn.GetNextEntry()), entry.get() != NULL)
			if (!entry->GetName().Matches(_("META-INF*")))
				if (!zipOut.CopyEntry(entry.release(), zipIn))
					break;
	}
	
	// Modify the jar
	TaskStep(); // STEP 2
	SetStatus(_("Installing mods - Adding mod files..."));
	for (ConstModIterator iter = modList->begin(); iter != modList->end(); iter++)
	{
		wxFileName modFileName = iter->GetFileName();
		SetStatus(_("Installing mods - Adding ") + modFileName.GetFullName());
		if (iter->IsZipMod())
		{
			wxFFileInputStream modStream(modFileName.GetFullPath());
			TransferZipArchive(modStream, zipOut);
		}
		else
		{
			wxFileName destFileName = modFileName;
			destFileName.MakeRelativeTo(m_inst->GetInstModsDir().GetFullPath());

			wxFFileInputStream input(modFileName.GetFullPath());
			zipOut.PutNextEntry(destFileName.GetFullPath());
			zipOut.Write(input);
		}
	}
	
	// Recompress the jar
	TaskStep(); // STEP 3
	SetStatus(_("Installing mods - Recompressing jar..."));

	m_inst->SetNeedsRebuild(false);
}


void ModderTask::TaskStep()
{
	step++;
	int p = (int)(((float)step / 4.0f) * 100);
	SetProgress(p);
}

void ModderTask::OnFail(const wxString &errorMsg)
{
	SetStatus(errorMsg);
	wxSleep(3);
	OnErrorMessage(errorMsg);
}
