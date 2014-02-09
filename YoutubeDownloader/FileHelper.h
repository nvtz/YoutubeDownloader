/***********************************************************
 *
 * FileHelper.h
 *
 *         Responsible for reading and writing the files
 *
 ***********************************************************/
#include "stdafx.h"

#ifndef FILEHELPER_H
#define FILEHELPER_H

class FileHelper
{
public:
    // Constructors and Destructors
    FileHelper();
    FileHelper(std::wstring& wstrUserAgent);
    ~FileHelper();

    // File Open APIs
    BOOL OpenFile(std::wstring& wstrFileName);
    BOOL NewFile(std::wstring& wstrFileName, BOOL bOverWrite = FALSE);
    BOOL CloseFile();
    
    // Read APIs
    HRESULT ReadByte(BYTE& byteData);
    HRESULT ReadBuffer(BYTE*& pbyteBuffer, UINT uiBufferLength);
    HRESULT ReadBuffer(std::string& strBuffer, UINT uiBufferLength);
    HRESULT ReadChar(TCHAR& wChar);
    HRESULT ReadBeforeTillMatchAnyOfThese(std::string& strByteChars, std::string& strOutBuffer);
    HRESULT ReadBeforeTillMatchThisPattern(std::string& strBytePattern, std::string& strOutBuffer, BOOL bExcludePatternReading = TRUE);
    BOOL    FindPattern(std::string& strPattern, BOOL bSearchFromStart, BOOL bDontReadPattern = TRUE);
    
    //Write APIs
    HRESULT WriteByte(BYTE byteData);
    HRESULT WriteBuffer(BYTE* pbyteBuffer, UINT uiBufferLength);
    HRESULT WriteChar(TCHAR wChar);

    // File Helper Functions
    BOOL Reset(BOOL bDeleteTempFile = FALSE);
    BOOL ResetFilePosition();
    BOOL SetFilePosition(ULONGLONG ullFilePos);
    BOOL GetFilePosition(ULONGLONG& ullFilePos);
    BOOL MoveForwardByNBytes(UINT uiNumberOfBytes);
    BOOL MoveBackwardByNBytes(UINT uiNumberOfBytes);
    BOOL GetFileSize(ULONGLONG& ullFileSize);

    // Create Temp File
    BOOL CreateTempFile();
    BOOL CreateTempFile(std::wstring& wstrFileExtension);
    BOOL GetFileName(std::wstring& wstrFileName);
    BOOL DeleteFile();

private:
    HANDLE m_hFile;
    std::wstring m_wstrFileName;
};

#endif