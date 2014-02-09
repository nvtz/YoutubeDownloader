// YoutubeDownloader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FileHelper.h"
#include "ContentDownloader.h"
#include "URLParser.h"
#include <iostream>

void TestFileHelperBVT();

BOOL CopyItToCurrentDirectory(std::wstring& wstrNewFileName, std::wstring& wstrTempFileName);

int _tmain(int argc, _TCHAR* argv[])
{
    int    iURLLen = 0;
    WCHAR* wchURL  = NULL;

    std::wstring wstrURL;
    std::wstring wstrTempFileName;
    std::wstring wstrFilePathName;
    std::wstring wstrTitle;
    std::wstring wstrFirst;
    std::wstring wstrSecond;

    if(argc <= 1)
    {
        printf("\n***********************************************************************");
        printf("\n");
        printf("\n\t YouTube Downloader");
        printf("\n");
        printf("\n Usage: %S YOU_TUBE_URL", argv[0]);
        printf("\n");
        printf("\n Eaxample: %S http://www.youtube.com/watch?v=YOUTUBE_VIDEO_ID", argv[0]);
        printf("\n");
        printf("\n***********************************************************************\n");
        exit(0);
    }
    else
    {
        printf("\n***********************************************************************");
        printf("\n");
        printf("\n\t YouTube Downloader");
        printf("\n");
        printf("\n***********************************************************************");
        printf("\n");
        printf("\n");
    }
    for(int i = 1; i <argc; i++)
    {
        BOOL bVDODownloaded = FALSE;
        iURLLen = wcslen(argv[i]);
        wchURL  = new WCHAR[iURLLen + 1];

        memset(wchURL, 0, (iURLLen + 1) * sizeof(WCHAR));
        wcsncpy(wchURL, argv[i], iURLLen);

        wstrTempFileName.clear();
        wstrFilePathName.clear();
        wstrTitle.clear();
        wstrFirst.clear();
        wstrSecond.clear();
        wstrURL.clear();
        wstrURL.assign(wchURL);

        printf("\n\nDownloading %S\n\n", wchURL);
        URLParser objURLParser;
        std::vector<KEY_VAL_PAIR> v_pairKeyVal;
        if(S_OK == objURLParser.Parse(wstrURL,v_pairKeyVal))
        {
            std::vector<KEY_VAL_PAIR>::iterator iter = v_pairKeyVal.begin();
            if(std::string::npos != ((*iter).first).find(L"Title"))
            {
                wstrTitle.assign((*iter).second);
                wstrFilePathName = wstrTitle;
                iter++;
            }
            for(; iter != v_pairKeyVal.end(); iter++)
            {
                wstrFirst.assign((*iter).first);
                wstrSecond.assign((*iter).second);
                if(std::wstring::npos != wstrFirst.find(L"Video"))
                {
                    ContentDownloader objContentDownloader;
                    if(S_OK == objContentDownloader.DownloadURL(wstrSecond, wstrTempFileName, TRUE))
                    {
                        if(TRUE == CopyItToCurrentDirectory(wstrFilePathName, wstrTempFileName))
                        {
                            printf("\n\nFile has been saved at:\n\t %S\n", wstrFilePathName.c_str());
                        }
                    
                        bVDODownloaded = TRUE;
                    }
                }
                if(TRUE == bVDODownloaded)
                {
                    break;
                }
            }
        }
        else
        {
            printf("Error: Unable to download video.\n");
        }
        printf("\n");
        printf("\n***********************************************************************\n\n");
        delete[] wchURL;
    }
	return 0;
}

BOOL CopyItToCurrentDirectory(std::wstring& wstrNewFileName, std::wstring& wstrTempFileName)
{
    BOOL bRetVal = FALSE;
    WCHAR wszCurrWorkingDir[MAX_PATH] = {0};
    std::wstring wstrCurrentWorkingDirectory;
    WCHAR wszTempDir[MAX_PATH] = {0};
    std::wstring wstrTempDirectory;
    std::wstring wstrFileExtension;
    std::wstring wstrNewFullPath;
    std::wstring wstrNewTempFullPath;
    if(wstrTempFileName.length() < 4)
    {
        return FALSE;
    }
    if(0 == GetCurrentDirectory(MAX_PATH, wszCurrWorkingDir))
    {
        return FALSE;
    }
    if(0 == GetTempPath(MAX_PATH, wszTempDir))
    {
        return FALSE;
    };
    wstrCurrentWorkingDirectory.assign(wszCurrWorkingDir);
    wstrCurrentWorkingDirectory.append(L"\\");
    wstrTempDirectory.assign(wszTempDir);
    wstrTempDirectory.append(L"\\");
    wstrFileExtension = wstrTempFileName.substr(wstrTempFileName.length() - 4); 
    wstrNewFileName.append(wstrFileExtension);
    wstrNewFullPath = wstrCurrentWorkingDirectory + wstrNewFileName;
    wstrNewTempFullPath = wstrTempDirectory + wstrNewFileName;
    if(TRUE == CopyFile(wstrTempFileName.c_str(), wstrNewFullPath.c_str(), FALSE))
    {
        DeleteFile(wstrTempFileName.c_str());
        bRetVal = TRUE;
    }
    else if(TRUE == MoveFile(wstrTempFileName.c_str(), wstrNewTempFullPath.c_str()))
    {
        bRetVal = TRUE;
    }
    return bRetVal;
}

void TestFileHelperBVT()
{
    DWORD dwRetVal = 0;
    TCHAR   wszCurrentDirectory[MAX_PATH] = {0};
    TCHAR   wszTempFileName[MAX_PATH]     = {0};
    LPCTSTR lpPrefixString                = L"YD_";
    dwRetVal = GetTempFileName(wszCurrentDirectory, lpPrefixString, 0, wszTempFileName);
    dwRetVal = GetTempPath(MAX_PATH, wszCurrentDirectory);
    std::wstring wstrFileName;
    wstrFileName.assign(wszCurrentDirectory);
    wstrFileName.append(wszTempFileName);
    
    FileHelper ofh;
    ofh.NewFile(wstrFileName, TRUE);
    ULONGLONG ullFileSize = 0;
    TCHAR wszBuffer[] = L"This is a test";
    ofh.WriteBuffer((BYTE*)wszBuffer, wcslen(wszBuffer) * sizeof(TCHAR));
    ofh.CloseFile();
    ofh.OpenFile(wstrFileName);
    if(!ofh.GetFileSize(ullFileSize))
    {
        std::cout<<"Unable to get filesize";
    }
    else
    {
        std::cout<<wstrFileName.c_str()<<std::endl;
        std::cout<<ullFileSize<<std::endl;
    }
    for(int i = 0 ; i <(int)ullFileSize; i++)
    {
        TCHAR wCh = L' ';
        ofh.ReadChar(wCh);
        printf("%c", wCh);
    }
    ofh.CloseFile();
    std::cout<<std::endl;
    
    DeleteFile(wstrFileName.c_str());
}
