/***********************************************************
 *
 * ContentDownloader.h
 *
 *         Responsible for download the contents
 *
 ***********************************************************/
#include "stdafx.h"
#include "FileHelper.h"

#ifndef CONTENTDOWNLOADER_H
#define CONTENTDOWNLOADER_H

// MACROS
#define FOUR_KB (4 * 1024)
#define DOWNLOAD_PROGRESS_BARS 50

class ContentDownloader
{
public:
    ContentDownloader();
    ContentDownloader(std::wstring& wstrUserAgent);
    ~ContentDownloader();
    BOOL Initialize(std::wstring& wstrUserAgent);
    BOOL Uninitialize();
    HRESULT DownloadURL(std::wstring& wstrURL, __out std::wstring& wstrFileName, BOOL bDisplayProgress = FALSE);

private:
    HRESULT ConnectToInternet(std::wstring& wstrServerURL);
    HRESULT Download(std::wstring& wstrServerURL, std::wstring& wstrFileToDownload, __out std::wstring& wstrFileName, BOOL bDisplayProgress = FALSE);
    void CloseAllHandles();
    void DisplayProgress(long lProgress, long lTotal);

private:
    std::wstring m_wstrURL;
    HINTERNET    m_hInternet;
    HINTERNET    m_hConnect;
    HINTERNET    m_hSession;
    HINTERNET    m_hRequest;
};

#endif