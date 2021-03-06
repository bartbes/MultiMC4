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

#include "instance.h"
#include <wx/filesys.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/dir.h>

#include "launcherdata.h"
#include "osutils.h"
#include <datautils.h>

DEFINE_EVENT_TYPE(wxEVT_INST_OUTPUT)

const wxString cfgFileName = _("instance.cfg");

bool IsValidInstance(wxFileName rootDir)
{
	return rootDir.DirExists() && wxFileExists(Path::Combine(rootDir, cfgFileName));
}

Instance *Instance::LoadInstance(wxFileName rootDir)
{
	if (IsValidInstance(rootDir))
		return new Instance(rootDir);
	else
		return NULL;
}

Instance::Instance(const wxFileName &rootDir)
{
	this->rootDir = rootDir;
	config = new wxFileConfig(wxEmptyString, wxEmptyString, GetConfigPath().GetFullPath(), wxEmptyString,
							  wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH);
	evtHandler = NULL;
	MkDirs();
	UpdateModList();
	LoadMLModList();
}

Instance::~Instance(void)
{
	delete config;
	if (IsRunning())
	{
		instProc->Detach();
	}
	Save();
}

// Makes ALL the directories! \o/
void Instance::MkDirs()
{
	if (!GetRootDir().DirExists())
		GetRootDir().Mkdir();
	
	if (!GetMCDir().DirExists())
		GetMCDir().Mkdir();
	if (!GetBinDir().DirExists())
		GetBinDir().Mkdir();
	if (!GetSavesDir().DirExists())
		GetSavesDir().Mkdir();
	if (!GetMLModsDir().DirExists())
		GetMLModsDir().Mkdir();
	if (!GetResourceDir().DirExists())
		GetResourceDir().Mkdir();
	if (!GetScreenshotsDir().DirExists())
		GetScreenshotsDir().Mkdir();
	if (!GetTexturePacksDir().DirExists())
		GetTexturePacksDir().Mkdir();
	
	if (!GetInstModsDir().DirExists())
		GetInstModsDir().Mkdir();
}

bool Instance::Save() const
{
// 	if (!GetRootDir().DirExists())
// 	{
// 		GetRootDir().Mkdir();
// 	}
// 
// 	wxFileName filename = GetConfigPath();
// 	using boost::property_tree::ptree;
// 	ptree pt;
// 
// 	pt.put<std::string>("name", stdStr(name));
// 	pt.put<std::string>("iconKey", stdStr(iconKey));
// 	pt.put<std::string>("notes", stdStr(notes));
// 	pt.put<bool>("NeedsRebuild", needsRebuild);
// 	pt.put<bool>("AskUpdate", askUpdate);
// 
// 	write_ini(stdStr(filename.GetFullPath()).c_str(), pt);
// 	return true;
	return false;
}

bool Instance::Load(bool loadDefaults)
{
// 	using boost::property_tree::ptree;
// 	ptree pt;
// 
// 	wxFileName filename = GetConfigPath();
// 	try
// 	{
// 		if (!loadDefaults)
// 			read_ini(stdStr(filename.GetFullPath()).c_str(), pt);
// 	}
// 	catch (boost::property_tree::ini_parser_error e)
// 	{
// 		wxLogError(_("Failed to parse instance config file '%s'. %s"), 
// 			stdStr(filename.GetFullPath()).c_str(),
// 			e.message().c_str());
// 		return false;
// 	}
// 
// 	name = wxStr(pt.get<std::string>("name", "Unnamed Instance"));
// 	iconKey = wxStr(pt.get<std::string>("iconKey", "default"));
// 	notes = wxStr(pt.get<std::string>("notes", ""));
// 	
// 	needsRebuild = pt.get<bool>("NeedsRebuild", false);
// 	askUpdate = pt.get<bool>("AskUpdate", true);
// 	return true;
	return false;
}

wxFileName Instance::GetRootDir() const
{
	return rootDir;
}

wxFileName Instance::GetConfigPath() const
{
	return wxFileName(rootDir.GetFullPath(), cfgFileName);
}

wxFileName Instance::GetMCDir() const
{
	wxFileName mcDir;
	
	if (ENUM_CONTAINS(wxPlatformInfo::Get().GetOperatingSystemId(), wxOS_MAC) || 
		wxFileExists(Path::Combine(GetRootDir(), _("minecraft"))))
	{
		mcDir = wxFileName::DirName(Path::Combine(GetRootDir(), _("minecraft")));
	}
	else
	{
		mcDir = wxFileName::DirName(Path::Combine(GetRootDir(), _(".minecraft")));
	}
	
	return mcDir;
}

wxFileName Instance::GetBinDir() const
{
	return wxFileName::DirName(GetMCDir().GetFullPath() + _("/bin"));
}

wxFileName Instance::GetMLModsDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("mods")));
}

wxFileName Instance::GetResourceDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("resources")));
}

wxFileName Instance::GetSavesDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("saves")));
}

wxFileName Instance::GetScreenshotsDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("screenshots")));
}

wxFileName Instance::GetTexturePacksDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("texturepacks")));
}


wxFileName Instance::GetInstModsDir() const
{
	return wxFileName::DirName(Path::Combine(GetRootDir().GetFullPath(), _("instMods")));
}

wxFileName Instance::GetVersionFile() const
{
	return wxFileName::FileName(GetBinDir().GetFullPath() + _("/version"));
}

wxFileName Instance::GetMCBackup() const
{
	return wxFileName::FileName(GetBinDir().GetFullPath() + _("/mcbackup.jar"));
}

wxFileName Instance::GetMCJar() const
{
	return wxFileName::FileName(GetBinDir().GetFullPath() + _("/minecraft.jar"));
}

wxFileName Instance::GetModListFile() const
{
	return wxFileName::FileName(Path::Combine(GetRootDir(), _("modlist")));
}


wxString Instance::ReadVersionFile()
{
	if (!GetVersionFile().FileExists())
		return _("");
	
	// Open the file for reading
	wxFSFile *vFile = wxFileSystem().OpenFile(GetVersionFile().GetFullPath(), wxFS_READ);
	wxString retVal;
	wxStringOutputStream outStream(&retVal);
	outStream.Write(*vFile->GetStream());
	wxDELETE(vFile);
	return retVal;
}

void Instance::WriteVersionFile(const wxString &contents)
{
	if (!GetBinDir().DirExists())
		GetBinDir().Mkdir();
	
	wxFile vFile;
	if (!vFile.Create(GetVersionFile().GetFullPath(), true))
		return;
	wxFileOutputStream outStream(vFile);
	wxStringInputStream inStream(contents);
	outStream.Write(inStream);
}


wxString Instance::GetName() const
{
	return GetSetting<wxString>(_("name"), _("Unnamed Instance"));
}

void Instance::SetName(wxString name)
{
	SetSetting<wxString>(_("name"), name);
}

wxString Instance::GetIconKey() const
{
	return GetSetting<wxString>(_("iconKey"), _("default"));
}

void Instance::SetIconKey(wxString iconKey)
{
	SetSetting<wxString>(_("iconKey"), iconKey);
}

wxString Instance::GetNotes() const
{
	return GetSetting<wxString>(_("notes"), _(""));
}

void Instance::SetNotes(wxString notes)
{
	SetSetting<wxString>(_("notes"), notes);
}

bool Instance::ShouldRebuild() const
{
	return GetSetting<bool>(_("NeedsRebuild"), false);
}

void Instance::SetNeedsRebuild(bool value)
{
	SetSetting<bool>(_("NeedsRebuild"), value);
}


wxProcess *Instance::Launch(wxString username, wxString sessionID, bool redirectOutput)
{
	if (username.IsEmpty())
		username = _("Offline");
	
	if (sessionID.IsEmpty())
		sessionID = _("Offline");
	
	ExtractLauncher();
	
	wxString javaPath = settings.GetJavaPath();
	wxString additionalArgs = settings.GetJvmArgs();
	int xms = settings.GetMinMemAlloc();
	int xmx = settings.GetMaxMemAlloc();
	wxFileName mcDirFN = GetMCDir().GetFullPath();
	mcDirFN.MakeAbsolute();
	wxString mcDir = mcDirFN.GetFullPath();
	wxString wdArg = wxGetCwd();
	
// 	if (IS_WINDOWS())
// 	{
	mcDir.Replace(_("\\"), _("\\\\"));
	wdArg.Replace(_("\\"), _("\\\\"));
// 	}
	
	wxString launchCmd = wxString::Format(_("\"%s\" %s -Xmx%im -Xms%im -cp \"%s\" MultiMCLauncher \"%s\" \"%s\" \"%s\""),
		javaPath.c_str(), additionalArgs.c_str(), xmx, xms, wdArg.c_str(), mcDir.c_str(), username.c_str(), sessionID.c_str());
	m_lastLaunchCommand = launchCmd;
	
	instProc = new wxProcess(this);
	
	if (redirectOutput)
		instProc->Redirect();
	
	instProc = wxProcess::Open(launchCmd, wxEXEC_ASYNC);
	m_running = true;
	
	return instProc;
}

void Instance::ExtractLauncher()
{
	wxMemoryInputStream launcherInputStream(MultiMCLauncher_class, MultiMCLauncher_class_len);
	wxFileOutputStream launcherOutStream(_("MultiMCLauncher.class"));
	launcherOutStream.Write(launcherInputStream);
}


void Instance::OnInstProcExited(wxProcessEvent& event)
{
	m_running = false;
	printf("Instance exited with code %i.\n", event.GetExitCode());
	if (evtHandler != NULL)
	{
		evtHandler->AddPendingEvent(event);
	}
}

void Instance::SetEvtHandler(wxEvtHandler* handler)
{
	evtHandler = handler;
}

bool Instance::IsRunning() const
{
	return m_running;
}

wxProcess* Instance::GetInstProcess() const
{
	return instProc;
}

wxString Instance::GetLastLaunchCommand() const
{
	return m_lastLaunchCommand;
}

void Instance::LoadModList()
{
	if (GetModListFile().FileExists())
	{
		wxFileInputStream inputStream(GetModListFile().GetFullPath());
		wxStringList modListFile = ReadAllLines(inputStream);
		
		for (wxStringList::iterator iter = modListFile.begin(); iter != modListFile.end(); iter++)
		{
			// Normalize the path to the instMods dir.
			wxFileName modFile(*iter);
			modFile.Normalize(wxPATH_NORM_ALL, GetInstModsDir().GetFullPath());
			modFile.MakeRelativeTo();
			
			if (!Any(modList.begin(), modList.end(), [&modFile] (Mod mod) -> bool
				{ return mod.GetFileName().SameAs(wxFileName(modFile)); }))
			{
				//SetNeedsRebuild();
				modList.push_back(Mod(modFile));
			}
		}
	}
	
	for (size_t i = 0; i < modList.size(); i++)
	{
		if (!modList[i].GetFileName().FileExists())
		{
			SetNeedsRebuild();
			modList.erase(modList.begin() + i);
			i--;
		}
	}
	
	LoadModListFromDir(GetInstModsDir());
}

void Instance::LoadModListFromDir(const wxFileName& dir, bool mlMod)
{
	ModList *list;
	
	if (mlMod)
		list = &mlModList;
	else
		list = &modList;
	
	wxDir modDir(dir.GetFullPath());
	
	if (!modDir.IsOpened())
	{
		wxLogError(_("Failed to open directory: ") + dir.GetFullPath());
		return;
	}
	
	wxString currentFile;
	if (modDir.GetFirst(&currentFile))
	{
		do
		{
			currentFile = Path::Combine(modDir.GetName(), currentFile);
			if (wxFileExists(currentFile) || mlMod)
			{
				Mod mod(currentFile);
				
				if (mlMod || !Any(list->begin(), list->end(), [&currentFile] (Mod mod) -> bool
					{ return mod.GetFileName().SameAs(wxFileName(currentFile)); }))
				{
					if (!mlMod)
						SetNeedsRebuild();

					list->push_back(mod);
				}
			}
			else if (wxDirExists(currentFile))
			{
				LoadModListFromDir(wxFileName(currentFile), mlMod);
			}
		} while (modDir.GetNext(&currentFile));
	}
}

void Instance::SaveModList()
{
	wxString text;
	for (ModIterator iter = modList.begin(); iter != modList.end(); iter++)
	{
		wxFileName modFile = iter->GetFileName();
		modFile.MakeRelativeTo(GetInstModsDir().GetFullPath());
		text.Append(modFile.GetFullPath());
		text.Append(_("\n"));
	}
	
	wxFile outFile(GetModListFile().GetFullPath(), wxFile::write);
	wxFileOutputStream out(outFile);
	WriteAllText(out, text);
}

void Instance::UpdateModList()
{
	LoadModList();
	SaveModList();
}

ModList *Instance::GetModList()
{
	return &modList;
}

void Instance::InsertMod(size_t index, const wxFileName &source)
{
	wxFileName dest(Path::Combine(GetInstModsDir().GetFullPath(), source.GetFullName()));
	if (!source.SameAs(dest))
	{
		wxCopyFile(source.GetFullPath(), dest.GetFullPath());
	}

	dest.MakeRelativeTo();
	SetNeedsRebuild();
	
	int oldIndex = Find(modList.begin(), modList.end(), [&dest] (Mod mod) -> bool
		{ return mod.GetFileName().SameAs(dest); });
	
	if (oldIndex != -1)
	{
		modList.erase(modList.begin() + oldIndex);
	}
	
	if (index >= modList.size())
		modList.push_back(Mod(dest));
	else
		modList.insert(modList.begin() + index, Mod(dest));
	
	SaveModList();
}

void Instance::DeleteMod(size_t index)
{
	Mod *mod = &modList[index];
	if (wxRemoveFile(mod->GetFileName().GetFullPath()))
	{
		SetNeedsRebuild();
		modList.erase(modList.begin() + index);
		SaveModList();
	}
	else
	{
		wxLogError(_("Failed to delete mod."));
	}
}

void Instance::DeleteMLMod(size_t index)
{
	Mod *mod = &mlModList[index];
	if (wxRemoveFile(mod->GetFileName().GetFullPath()))
	{
		mlModList.erase(mlModList.begin() + index);
	}
	else
	{
		wxLogError(_("Failed to delete mod."));
	}
}

ModList *Instance::GetMLModList()
{
	return &mlModList;
}

void Instance::LoadMLModList()
{
	mlModList.clear();
	//for (size_t i = 0; i < mlModList.size(); i++)
	//{
	//	if (!mlModList[i].GetFileName().FileExists())
	//	{
	//		mlModList.erase(mlModList.begin() + i);
	//		i--;
	//	}
	//}
	
	LoadModListFromDir(GetMLModsDir(), true);
}

template <typename T>
T Instance::GetSetting(const wxString &key, T defValue) const
{
	T val;
	if (config->Read(key, &val))
		return val;
	else
		return defValue;
}

template <typename T>
void Instance::SetSetting(const wxString &key, T value, bool suppressErrors)
{
	if (!config->Write(key, value) && !suppressErrors)
		wxLogError(_("Failed to write config setting %s"), key.c_str());
	config->Flush();
}

wxFileName Instance::GetSetting(const wxString &key, wxFileName defValue) const
{
	wxString val;
	if (config->Read(key, &val))
	{
		if (defValue.IsDir())
			return wxFileName::DirName(val);
		else
			return wxFileName::FileName(val);
	}
	else
		return defValue;
}

void Instance::SetSetting(const wxString &key, wxFileName value, bool suppressErrors)
{
	if (!config->Write(key, value.GetFullPath()) && !suppressErrors)
		wxLogError(_("Failed to write config setting %s"), key.c_str());
	config->Flush();
}


BEGIN_EVENT_TABLE(Instance, wxEvtHandler)
	EVT_END_PROCESS(wxID_ANY, Instance::OnInstProcExited)
END_EVENT_TABLE()