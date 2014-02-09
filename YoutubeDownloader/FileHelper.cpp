/***********************************************************
 *
 * ContentDownloader.cpp
 *
 *         Responsible for download the contents
 *
 ***********************************************************/
#include "FileHelper.h"
#include <iostream>

// Constructors and Destructors
FileHelper::FileHelper()
    :m_hFile(INVALID_HANDLE_VALUE)
{
}

FileHelper::FileHelper(std::wstring& wstrUserAgent)
    :m_hFile(INVALID_HANDLE_VALUE)
{
}

FileHelper::~FileHelper()
{
    if((INVALID_HANDLE_VALUE == m_hFile) || (NULL == m_hFile))
    {
        return;
    }
    CloseFile();
}

// File Open APIs
BOOL FileHelper::OpenFile(std::wstring& wstrFileName)
{
    BOOL bResult = FALSE;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        return FALSE;
    }
    m_hFile = CreateFile(wstrFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
    DWORD deError = GetLastError();
    VERIFY_BOOL(INVALID_HANDLE_VALUE != m_hFile);
    m_wstrFileName.assign(wstrFileName);
    bResult = TRUE;

cleanup:

    return bResult;
}
BOOL FileHelper::NewFile(std::wstring& wstrFileName, BOOL bOverWrite)
{
    BOOL  bResult               = FALSE;
    DWORD dwCreationDisposition = CREATE_NEW;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        return FALSE;
    }
    if(TRUE == bOverWrite)
    {
        dwCreationDisposition = CREATE_ALWAYS;
    }
    m_hFile = CreateFile(wstrFileName.c_str(), GENERIC_ALL, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL , NULL);
    DWORD deError = GetLastError();
    VERIFY_BOOL(INVALID_HANDLE_VALUE != m_hFile);
    m_wstrFileName.assign(wstrFileName);
    bResult = TRUE;

cleanup:

    return bResult;
}
BOOL FileHelper::CloseFile()
{
    if((INVALID_HANDLE_VALUE == m_hFile) || (NULL == m_hFile))
    {
        return FALSE;
    }
    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    m_wstrFileName.clear();
    return TRUE;
}
// Read APIs
HRESULT FileHelper::ReadByte(BYTE& byteData)
{
    HRESULT hr          = E_FAIL;
    BOOL    bRetVal     = FALSE;
    DWORD   dwBytesRead = 0;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        bRetVal = ReadFile(m_hFile, &byteData ,1, &dwBytesRead ,NULL);
        bRetVal = TRUE;
    }
    hr = BOOL2HR(bRetVal);
    return hr;
}
HRESULT FileHelper::ReadBuffer(BYTE*& pbyteBuffer, UINT uBufferLength)
{
    HRESULT hr          = E_FAIL;
    BOOL    bRetVal     = FALSE;
    DWORD   dwBytesRead = 0;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        bRetVal = ReadFile(m_hFile, pbyteBuffer, uBufferLength, &dwBytesRead ,NULL);
        bRetVal = TRUE;
    }
    hr = BOOL2HR(bRetVal);
    return hr;
}

HRESULT FileHelper::ReadBuffer(std::string& strBuffer, UINT uiBufferLength)
{
    HRESULT hr          = E_FAIL;
    BOOL    bRetVal     = FALSE;
    DWORD   dwBytesRead = 0;
    BYTE*   pbyteBuffer = NULL;
    if(0 == uiBufferLength)
    {
        return E_FAIL;
    }
    pbyteBuffer = new BYTE[uiBufferLength + 1];
    memset(pbyteBuffer, 0, uiBufferLength + 1);
    hr = ReadBuffer(pbyteBuffer, uiBufferLength);
    if(S_OK == hr)
    {
        strBuffer.assign((char*)pbyteBuffer);
    }
    delete[] pbyteBuffer;
    return hr;
}
HRESULT FileHelper::ReadChar(TCHAR& wChar)
{
    HRESULT hr          = E_FAIL;
    BOOL    bRetVal     = FALSE;
    DWORD   dwBytesRead = 0;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        bRetVal = ReadFile(m_hFile, &wChar, sizeof(TCHAR), &dwBytesRead ,NULL);
        bRetVal = TRUE;
    }
    hr = BOOL2HR(bRetVal);
    return hr;
}

HRESULT FileHelper::ReadBeforeTillMatchAnyOfThese(std::string& strByteChars, std::string& strOutBuffer)
{
    HRESULT hr     = E_FAIL;
    BYTE byteData = 0;
    while(S_OK == ReadByte(byteData))
    {
        BOOL bBreakWhileLoop = FALSE;
        std::string::iterator iter = strByteChars.begin();
        // if in any of these
        for(;iter != strByteChars.end(); iter++)
        {
            if((CHAR)(*iter) == (CHAR)byteData)
            {
                bBreakWhileLoop = TRUE;
                break;
            }
        }
        if(TRUE == bBreakWhileLoop)
        {
            break;
        }
        strOutBuffer.push_back(byteData);
    }
    if(0 < strOutBuffer.length())
    {
        hr = S_OK;
    }
    return hr;
}

HRESULT FileHelper::ReadBeforeTillMatchThisPattern(std::string& strBytePattern, std::string& strOutBuffer, BOOL bExcludePatternReading)
{
    BOOL      bRetVal         = TRUE;
    HRESULT   hr              = E_FAIL;
    CHAR*     pszToCompare    = NULL;
    CHAR*     pszCompare      = pszToCompare;
    BOOL      bImageFound     = FALSE;
    LONG      lCurFilePointer = 0;
    BYTE      byteData        = 0;
    ULONGLONG ullFilePos      = 0L;
    std::string strTempStr;
    if(0 > strBytePattern.length())
    {
        return E_FAIL;
    }
    // try to allocate memory for a pattern
    try
    {
        pszToCompare = new CHAR[strBytePattern.length() + 1];
        memset(pszToCompare, 0, strBytePattern.length() + 1);
        strncpy(pszToCompare, strBytePattern.c_str(), strBytePattern.length());
        pszCompare = pszToCompare;
    }
    catch(std::bad_alloc& e)
    {
        return E_FAIL;
    }

    while(true)
    {
        hr = ReadByte(byteData);
        if(S_OK != hr)
        {
            return E_FAIL;
        }
        strTempStr.push_back(byteData);
        bRetVal = GetFilePosition(ullFilePos) ;
        if(TRUE != bRetVal)
        {
            break;
        }
        
        //If it matches the first character for Pattern to find
        if((S_OK == hr) && ((byteData) == *pszCompare))
        {
            std::string strTempLoop;
            while(true)
            {
                //Get character from file
                hr = ReadByte(byteData);
                strTempLoop.push_back(byteData);
                int left = tolower(byteData);
                int right = tolower(*(++pszCompare));
                if((S_OK != hr) || ((left) != right))
                {
                    break;
                }
                //printf("%c", (byteData));
            }
            //If pattern to match pointer is not pointing to End of string
            if(!(*pszCompare == '\0'))
            {
                //strTempStr.clear();
                //Reinitialize the pattern matching pointer to start of string
                pszCompare = pszToCompare;
                SetFilePosition(ullFilePos);
            }
            else
            {
                strTempStr.append(strTempLoop);
                int iStrLen   = strTempStr.length();
                int iBytesLen = strBytePattern.length();
                if(0 < iStrLen)
                {
                    strTempStr.erase(iStrLen - 1);
                    if(TRUE == bExcludePatternReading)
                    {
                        strTempStr.erase(iStrLen - (1 + iBytesLen));
                    }
                    strOutBuffer.assign(strTempStr);
                }
                MoveBackwardByNBytes(1);
                if(TRUE == bExcludePatternReading)
                {
                    MoveBackwardByNBytes(iBytesLen);
                }
                // Pattern Found
                return S_OK;
            }
        }
    }

    return hr;
}

BOOL FileHelper::FindPattern(std::string& strPattern, BOOL bSearchFromStart, BOOL bDontReadPattern)
{
    BOOL      bRetVal         = FALSE;
    HRESULT   hr              = S_OK;
    CHAR*     pszToCompare    = NULL;
    CHAR*     pszCompare      = pszToCompare;
    BOOL      bImageFound     = FALSE;
    LONG      lCurFilePointer = 0;
    BYTE      byteData        = 0;
    ULONGLONG ullFilePos      = 0L;
    if(0 > strPattern.length())
    {
        return FALSE;
    }
    if(TRUE == bSearchFromStart)
    {
        ResetFilePosition();
    }
    // try to allocate memory for a pattern
    try
    {
        pszToCompare = new CHAR[strPattern.length() + 1];
        memset(pszToCompare, 0, strPattern.length() + 1);
        strncpy(pszToCompare, strPattern.c_str(), strPattern.length());
        pszCompare = pszToCompare;
    }
    catch(std::bad_alloc& e)
    {
        return FALSE;
    }

    while(true)
    {
        ULONGLONG ullFilePosition;
        GetFilePosition(ullFilePosition);
        //printf("\nPos: %ld", ullFilePosition);
        hr = ReadByte(byteData);
        if(S_OK != hr)
        {
            return FALSE;
        }
        bRetVal = GetFilePosition(ullFilePos) ;
        if(TRUE != bRetVal)
        {
            break;
        }
        ULONGLONG ullFileSize;
        GetFileSize(ullFileSize);
        //printf("\nPos: %ld", ullFileSize);
        if(ullFilePos == ullFileSize)
        {
            return FALSE;
        }

        //printf("%c", (byteData));
        
        //If it matches the first character for Pattern to find
        if((S_OK == hr) && ((byteData) == *pszCompare))
        {
            while(true)
            {
                //Get character from file
                hr = ReadByte(byteData);
                int left = tolower(byteData);
                int right = tolower(*(++pszCompare));
                if((S_OK != hr) || ((left) != right))
                {
                    break;
                }
                //printf("%c", (byteData));
            }
            //If pattern to match pointer is not pointing to End of string
            if(!(*pszCompare == '\0'))
            {
                //Reinitialize the pattern matching pointer to start of string
                pszCompare = pszToCompare;
                SetFilePosition(ullFilePos);
            }
            else
            {
                MoveBackwardByNBytes(1);
                int iBytesLen = strPattern.length();
                if(TRUE == bDontReadPattern)
                {
                    MoveBackwardByNBytes(iBytesLen);
                }
                // Pattern Found
                return TRUE;
            }
        }
    }
    return bRetVal;
}

//Write APIs
HRESULT FileHelper::WriteByte(BYTE byteData)
{
    HRESULT hr          = E_FAIL;
    BOOL    bRetVal     = FALSE;
    DWORD   dwBytesWritten = 0;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        bRetVal = WriteFile(m_hFile, &byteData, sizeof(TCHAR), &dwBytesWritten ,NULL);
        bRetVal = TRUE;
    }
    hr = BOOL2HR(bRetVal);
    return hr;
}
HRESULT FileHelper::WriteBuffer(BYTE* pbyteBuffer, UINT uiBufferLength)
{
    HRESULT hr          = E_FAIL;
    BOOL    bRetVal     = FALSE;
    DWORD   dwBytesWritten = 0;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        bRetVal = WriteFile(m_hFile, pbyteBuffer, uiBufferLength, &dwBytesWritten ,NULL);
        //std::cout <<std::endl<<dwBytesWritten<<std::endl;
        bRetVal = TRUE;
    }
    hr = BOOL2HR(bRetVal);
    return hr;
}
HRESULT FileHelper::WriteChar(TCHAR wChar)
{
    HRESULT hr             = E_FAIL;
    BOOL    bRetVal        = FALSE;
    DWORD   dwBytesWritten = 0;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        bRetVal = WriteFile(m_hFile, &wChar, sizeof(TCHAR), &dwBytesWritten ,NULL);
        bRetVal = TRUE;
    }
    hr = BOOL2HR(bRetVal);
    return hr;
}

BOOL FileHelper::Reset(BOOL bDeleteFile)
{
    if(TRUE == bDeleteFile)
    {
        return DeleteFile();
    }
    return CloseFile();
}

BOOL FileHelper::ResetFilePosition()
{
    return SetFilePosition((ULONGLONG)0);
}

// File Helper Functions
BOOL FileHelper::SetFilePosition(ULONGLONG ullFilePos)
{
    LARGE_INTEGER liFilePos    = {0};
    liFilePos.QuadPart         = ullFilePos;
    LONG  lDistanceToMoveHigh  = liFilePos.HighPart;
    DWORD dwDistanceToMoveLow  = liFilePos.LowPart;
    DWORD dwPositionLow        = 0;
    BOOL  bRetVal              = FALSE;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        dwPositionLow = SetFilePointer(m_hFile, dwDistanceToMoveLow, &lDistanceToMoveHigh, FILE_BEGIN);
        if(INVALID_SET_FILE_POINTER == dwPositionLow)
        {
            return FALSE;
        }
        bRetVal = TRUE;
    }
    return bRetVal;
}

BOOL FileHelper::GetFilePosition(ULONGLONG& ullFilePos)
{
    LARGE_INTEGER liFilePos    = {0};
    LONG  lDistanceToMoveHigh  = 0;
    DWORD dwDistanceToMoveLow  = 0;
    DWORD dwPositionLow        = 0;
    BOOL  bRetVal              = FALSE;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        dwPositionLow = SetFilePointer(m_hFile, 0, &lDistanceToMoveHigh, FILE_CURRENT);
        if(INVALID_SET_FILE_POINTER == dwPositionLow)
        {
            return FALSE;
        }
        liFilePos.LowPart  = dwPositionLow;
        liFilePos.HighPart = lDistanceToMoveHigh;
        ullFilePos = liFilePos.QuadPart;
        bRetVal = TRUE;
    }
    return bRetVal;
}
BOOL FileHelper::MoveForwardByNBytes(UINT uNumberOfBytes)
{
    DWORD dwDistanceToMoveLow  = uNumberOfBytes;
    DWORD dwPositionLow        = 0;
    BOOL  bRetVal              = FALSE;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        dwPositionLow = SetFilePointer(m_hFile, dwDistanceToMoveLow, NULL, FILE_CURRENT);
        if(INVALID_SET_FILE_POINTER == dwPositionLow)
        {
            return FALSE;
        }
        bRetVal = TRUE;
    }
    return bRetVal;
}
BOOL FileHelper::MoveBackwardByNBytes(UINT uNumberOfBytes)
{
    DWORD dwDistanceToMoveLow  = uNumberOfBytes * -1;
    DWORD dwPositionLow        = 0;
    BOOL  bRetVal              = FALSE;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        dwPositionLow = SetFilePointer(m_hFile, dwDistanceToMoveLow, NULL, FILE_CURRENT);
        if(INVALID_SET_FILE_POINTER == dwPositionLow)
        {
            return FALSE;
        }
        bRetVal = TRUE;
    }
    return bRetVal;
}

BOOL FileHelper::GetFileSize(ULONGLONG& ullFileSize)
{
    LARGE_INTEGER liFileSize     = {0};
    DWORD         dwFileSizeHigh = 0;
    DWORD         dwFileSizeLow  = 0;
    BOOL          bRetVal        = FALSE;
    if((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
    {
        dwFileSizeLow = ::GetFileSize(m_hFile, &dwFileSizeHigh);
        if(INVALID_FILE_SIZE == dwFileSizeLow)
        {
            return FALSE;
        }
        liFileSize.LowPart = dwFileSizeLow;
        liFileSize.HighPart = dwFileSizeHigh;
        ullFileSize = liFileSize.QuadPart;
        bRetVal = TRUE;
    }
    return bRetVal;
}

BOOL FileHelper::CreateTempFile(std::wstring& wstrFileExtension)
{
    BOOL bRetVal = FALSE;
    DWORD dwRetVal = 0;
    TCHAR   wszCurrentDirectory[MAX_PATH] = {0};
    TCHAR   wszTempFileName[MAX_PATH]     = {0};
    LPCTSTR lpPrefixString                = L"YD_";
    std::wstring wstrTempFileName;
    // Delete Existing Temp File
    DeleteFile();
    // Get Temp Path Name
    dwRetVal = GetTempPath(MAX_PATH, wszCurrentDirectory);
    VERIFY_BOOL(0 != dwRetVal);
    // Get Temp File Name
    dwRetVal = GetTempFileName(wszCurrentDirectory, lpPrefixString, 0, wszTempFileName);
    VERIFY_BOOL(0 != dwRetVal);
    if(0 != wstrFileExtension.length())
    {
        // Delete the Temporary File
        ::DeleteFile(wszTempFileName);
        // String File Name
        wstrTempFileName.append(wszTempFileName);
        wstrTempFileName.append(wstrFileExtension);
    }
    else
    {
        wstrTempFileName.append(wszTempFileName);
    }
    bRetVal = NewFile(wstrTempFileName, TRUE);
cleanup:
    return bRetVal;
}

BOOL FileHelper::CreateTempFile()
{
    BOOL bRetVal = FALSE;
    DWORD dwRetVal = 0;
    TCHAR   wszCurrentDirectory[MAX_PATH] = {0};
    TCHAR   wszTempFileName[MAX_PATH]     = {0};
    LPCTSTR lpPrefixString                = L"YD_";
    std::wstring wstrTempFileName;
    // Delete Existing Temp File
    DeleteFile();
    // Get Temp Path Name
    dwRetVal = GetTempPath(MAX_PATH, wszCurrentDirectory);
    VERIFY_BOOL(0 != dwRetVal);
    // Get Temp File Name
    dwRetVal = GetTempFileName(wszCurrentDirectory, lpPrefixString, 0, wszTempFileName);
    VERIFY_BOOL(0 != dwRetVal);
    // String File Name
    wstrTempFileName.append(wszTempFileName);
    bRetVal = NewFile(wstrTempFileName, TRUE);
cleanup:
    return bRetVal;
}

BOOL FileHelper::DeleteFile()
{
    if((INVALID_HANDLE_VALUE == m_hFile) || (NULL == m_hFile))
    {
        return FALSE;
    }
    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    if(0 != m_wstrFileName.length())
    {
        ::DeleteFile(m_wstrFileName.c_str());
        m_wstrFileName.clear();
    }
    return TRUE;
}

BOOL FileHelper::GetFileName(std::wstring& wstrFileName)
{
    BOOL bRetVal = FALSE;
    if(0 < m_wstrFileName.length())
    {
        wstrFileName.assign(m_wstrFileName);
        bRetVal = TRUE;
    }
    return bRetVal;
}