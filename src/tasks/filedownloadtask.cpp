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

#include "filedownloadtask.h"
#include "curlutils.h"
#include <apputils.h>
#include <wx/wfstream.h>

FileDownloadTask::FileDownloadTask(const wxString &src, const wxFileName &dest, const wxString &message)
	: Task()
{
	m_src = src;
	m_dest = dest;
	m_message = message;
	successful = false;
}

void FileDownloadTask::TaskStart()
{
	if (m_message.IsEmpty())
		SetStatus(wxString::Format(_("Downloading file from %s..."), m_src.c_str()));
	else
		SetStatus(m_message);
	
	double downloadSize = GetDownloadSize();
	
	if (downloadSize < 0)
	{
		successful = false;
		wxString baseError = _("An error occurred when downloading the file.\n");
		switch (-(int)downloadSize)
		{
		case 404:
			OnErrorMessage(baseError + _("Error 404, the page could not be found."));
			return;
			
		default:
			OnErrorMessage(baseError + wxString::Format(_("Unknown error %i occurred."), -(int)downloadSize));
			return;
		}
	}
	
	CURL *curl = curl_easy_init();
	
	curl_easy_setopt(curl, CURLOPT_URL, TOASCII(m_src));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlLambdaCallback);
	
	size_t downloadedSize;
	wxFFileOutputStream outStream(m_dest.GetFullPath());
	CurlLambdaCallbackFunction curlWrite = [&] (void *buffer, size_t size) -> size_t
	{
		outStream.Write(buffer, size);
		downloadedSize += outStream.LastWrite();
		SetProgress(((double)downloadedSize / (double)downloadSize) * 100);
		return outStream.LastWrite();
	};
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlWrite);
	
	int curlErr = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	if (curlErr != 0)
	{
		OnErrorMessage(_("Download failed."));
		successful = false;
	}
	else
	{
		successful = true;
	}
}

double FileDownloadTask::GetDownloadSize()
{
	CURL *curl = curl_easy_init();
	
	curl_easy_setopt(curl, CURLOPT_URL, TOASCII(m_src));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlBlankCallback);
	curl_easy_setopt(curl, CURLOPT_NOBODY, true);
	
	if (curl_easy_perform(curl) != 0)
		return -1;
	
	long responseCode = 0;
	double contentLen = 0;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLen);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	
	curl_easy_cleanup(curl);
	
	if (responseCode == 404 || responseCode == 403 || responseCode == 500)
		return -responseCode;
	else
		return contentLen;
}
