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

#include "fsutils.h"
#include "apputils.h"
#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filesys.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

#if MSVC
#include <memory>
#else
#include <auto_ptr.h>
#endif


bool RecursiveDelete(const wxString &path)
{
	if (wxFileExists(path))
	{
		if (!wxRemoveFile(path))
			return false;
	}
	else if (wxDirExists(path))
	{
		wxDir dir(path);
		wxString subpath;
		
		if (dir.GetFirst(&subpath))
		{
			do
			{
				if (!RecursiveDelete(Path::Combine(path, subpath)))
					return false;
			} while (dir.GetNext(&subpath));
		}
		
		if (!wxRmdir(path))
			return false;
	}
	return true;
}

void ExtractZipArchive(wxInputStream &stream, const wxString &dest)
{
	wxZipInputStream zipStream(stream);
	std::auto_ptr<wxZipEntry> entry;
	while (entry.reset(zipStream.GetNextEntry()), entry.get() != NULL)
	{
		if (entry->IsDir())
			continue;
		
		wxString name = entry->GetName();
		wxFileName destFile(dest + (name.StartsWith(_("/")) ? _("") : _("/")) + name);
		
		destFile.Mkdir(0777, wxPATH_MKDIR_FULL);
		
		if (destFile.FileExists())
			wxRemoveFile(destFile.GetFullPath());
		
		wxFFileOutputStream outStream(destFile.GetFullPath());
		outStream.Write(zipStream);
		
// 		wxFFile file(destFile.GetFullPath(), _("w"));
// 		
// 		const size_t bufSize = 1024;
// 		void *buffer = new char[bufSize];
// 		while (!zipStream.Eof())
// 		{
// 			zipStream.Read(buffer, bufSize);
// 			file.Write(buffer, bufSize);
// 		}
// 		file.Flush();
	}
}

void TransferZipArchive(wxInputStream &stream, wxZipOutputStream &out)
{
	wxZipInputStream zipStream(stream);
	std::auto_ptr<wxZipEntry> entry;
	while (entry.reset(zipStream.GetNextEntry()), entry.get() != NULL)
	{
		if (entry->IsDir())
			continue;

		wxString name = entry->GetName();

		out.PutNextEntry(name);
		out.Write(zipStream);
	}
}

bool CompressRecursively(const wxString &path, wxZipOutputStream &zipStream, const wxString &topDir)
{
	wxFileName destPath(path);
	destPath.MakeRelativeTo(topDir);
	
	if (wxFileExists(path))
	{
		zipStream.PutNextEntry(destPath.GetFullPath());
		
		wxFFileInputStream inStream(path);
		zipStream.Write(inStream);
	}
	else if (wxDirExists(path))
	{
		zipStream.PutNextDirEntry(destPath.GetFullPath());
		wxDir dir(path);
		
		wxString subpath;
		if (dir.GetFirst(&subpath))
		{
			do
			{
				if (!CompressRecursively(Path::Combine(path, subpath), zipStream, topDir))
					return false;
			} while (dir.GetNext(&subpath));
		}
	}
	return true;
}

bool CompressZipArchive(wxOutputStream &stream, const wxString &srcDir)
{
	wxZipOutputStream zipStream(stream);
	return CompressRecursively(srcDir, zipStream, srcDir);
}

bool CreateAllDirs(const wxFileName &dir)
{
	if (!wxDirExists(Path::GetParent(dir.GetFullPath())))
	{
		if (!CreateAllDirs(Path::GetParent(dir.GetFullPath())))
			return false;
	}
	return wxMkdir(dir.GetFullPath());
}
