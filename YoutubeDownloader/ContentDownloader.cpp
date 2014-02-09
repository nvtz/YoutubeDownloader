/***********************************************************
 *
 * ContentDownloader.cpp
 *
 *         Responsible for download the contents
 *
 ***********************************************************/
#include "ContentDownloader.h"

// Default constructor
ContentDownloader::ContentDownloader()
    :m_hInternet(NULL), m_hConnect(NULL), m_hSession(NULL), m_hRequest(NULL)
{
    std::wstring wstrDefaultUserAgent(L"Mozilla/5.0 (Windows; U; Windows NT 6.0; en-US; rv:1.9.1.2) Gecko/20090729 Firefox/3.5.2");
    // Open Internet Connection
    Initialize(wstrDefaultUserAgent);
}

// Constructor with the string
ContentDownloader::ContentDownloader(std::wstring& wstrUserAgent)
    :m_hInternet(NULL), m_hConnect(NULL), m_hSession(NULL), m_hRequest(NULL)
{
    // Open Internet Connection
    Initialize(wstrUserAgent);
}

// Destructor
ContentDownloader::~ContentDownloader()
{
    if(NULL != m_hInternet)
    {
        Uninitialize();
    }
}

// Initialization with an argument
BOOL ContentDownloader::Initialize(std::wstring& wstrUserAgent)
{
    BOOL bResult = TRUE;
    // Open Internet Connection
    m_hInternet = WinHttpOpen( wstrUserAgent.c_str(),
                              WINHTTP_ACCESS_TYPE_NO_PROXY,
                              WINHTTP_NO_PROXY_NAME, 
                              WINHTTP_NO_PROXY_BYPASS, 0 );

    bResult = (m_hInternet != NULL)? TRUE: FALSE;
    return bResult;
}

// Initialization
BOOL ContentDownloader::Uninitialize()
{
    BOOL bResult = TRUE;
    CloseAllHandles();
    return bResult;
}

HRESULT ContentDownloader::DownloadURL(std::wstring& wstrURL, __out std::wstring& wstrFileName, BOOL bDisplayProgress)
{
    HRESULT hr = E_FAIL;
    std::wstring wstrServerName;
    std::wstring wstrFileToDownload;
    if(wstrURL.length() == 0)
    {
        return E_INVALIDARG;
    }

    int iStartingOfServerName = 0;
    int iEndOfServerName = 0;
    if(0 == wstrURL.find(L"http:", 0))
    {
        iStartingOfServerName = 7; // 0 + lengthof "http://"
    }
    iEndOfServerName = wstrURL.find(L"/", iStartingOfServerName);
    if(std::string::npos != iEndOfServerName)
    {
        wstrServerName = wstrURL.substr(iStartingOfServerName, (iEndOfServerName - iStartingOfServerName));
    }
    wstrFileToDownload = wstrURL.substr(iEndOfServerName);
    // Connect to ServerName
    hr = ConnectToInternet(wstrServerName);
    VERIFY_HRESULT(hr);
    // Download the File
    hr = Download(wstrServerName, wstrFileToDownload, wstrFileName, bDisplayProgress);
    VERIFY_HRESULT(hr);

cleanup:

    return hr;
}

HRESULT ContentDownloader::ConnectToInternet(std::wstring& wstrServerURL)
{
    HRESULT hr = E_FAIL;
    // Connect to internet HTTP service for a given URL
    m_hConnect = WinHttpConnect( m_hInternet, wstrServerURL.c_str(),
                                    INTERNET_DEFAULT_HTTP_PORT, 0 );
    if (NULL == m_hConnect)
    {
        //If connection failed then exit function with failed error code
        hr = E_FAIL;
    }

    hr = S_OK;
    return hr;
}

HRESULT ContentDownloader::Download(std::wstring& wstrServerURL, std::wstring& wstrFileToDownload, __out std::wstring& wstrFileName, BOOL bDisplayProgress)
{
    HRESULT  hr                = E_FAIL;
    DWORD    dwSize            = 0;
    WCHAR*   wchOutputBuffer   = NULL;
    LONG     lFileSize         = -1;
    LONG     lCurrFileProgress = 0;
    std::wstring wstrFileExtenstion;
    //HTTP Request to open the file to be downloadedwstr
    m_hRequest = WinHttpOpenRequest( m_hConnect, L"GET",
                                   wstrFileToDownload.c_str(),
                                   NULL, WINHTTP_NO_REFERER, 
                                   WINHTTP_DEFAULT_ACCEPT_TYPES, 
                                   WINHTTP_FLAG_REFRESH );
    
    if (NULL == m_hRequest )
    {
        //If file not present or some other error which cause error to open file,
        //then exit with failed error code
        return E_FAIL;
    }

    BOOL bResult = WinHttpSendRequest( m_hRequest,
                                        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                        WINHTTP_NO_REQUEST_DATA, 0, 
                                        0, 0 );
 
    if(!bResult)
    {
        return E_FAIL;
    }

    bResult = WinHttpReceiveResponse( m_hRequest, NULL );

    if(!bResult)
    {
        return E_FAIL;
    }
    // Content TYPE
    WinHttpQueryHeaders( m_hRequest, WINHTTP_QUERY_CONTENT_TYPE,
                             WINHTTP_HEADER_NAME_BY_INDEX, NULL, 
                             &dwSize, WINHTTP_NO_HEADER_INDEX);
	
    // Allocate memory for the buffer.
    if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
    {
        wchOutputBuffer = new WCHAR[dwSize / sizeof(WCHAR)];
        memset(wchOutputBuffer, 0, (dwSize / sizeof(WCHAR)));
        // Content TYPE.
        bResult = WinHttpQueryHeaders(m_hRequest, 
                                      WINHTTP_QUERY_CONTENT_TYPE,
                                      WINHTTP_HEADER_NAME_BY_INDEX, 
                                      wchOutputBuffer, &dwSize, 
                                      WINHTTP_NO_HEADER_INDEX);
        if(!bResult)
        {
            delete[] wchOutputBuffer;
            return FALSE;
        }
        if(0 == wcsncmp(wchOutputBuffer, L"video/x-flv", wcslen(L"video/x-flv")))
        {
            wstrFileExtenstion.append(L".flv");
        }
        else if(0 == wcsncmp(wchOutputBuffer, L"video/mp4", wcslen(L"video/mp4")))
        {
            wstrFileExtenstion.append(L".mp4");
        }
        delete[] wchOutputBuffer;
        // Content LENGTH
        WinHttpQueryHeaders( m_hRequest, WINHTTP_QUERY_CONTENT_LENGTH,
                            WINHTTP_HEADER_NAME_BY_INDEX, NULL, 
                            &dwSize, WINHTTP_NO_HEADER_INDEX);
        // Content LENGTH
        if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
        {
            wchOutputBuffer = new WCHAR[dwSize / sizeof(WCHAR)];
            memset(wchOutputBuffer, 0, (dwSize / sizeof(WCHAR)));
            bResult = WinHttpQueryHeaders(m_hRequest, 
                                          WINHTTP_QUERY_CONTENT_LENGTH,
                                          WINHTTP_HEADER_NAME_BY_INDEX, 
                                          wchOutputBuffer, &dwSize, 
                                          WINHTTP_NO_HEADER_INDEX);
            if(!bResult)
            {
                delete[] wchOutputBuffer;
                return FALSE;
            }
            lFileSize = _wtol(wchOutputBuffer);
        }
    }

    FileHelper objFileHelper;
    if(TRUE != objFileHelper.CreateTempFile(wstrFileExtenstion))
    {
        return E_FAIL;
    }

    std::wstring wstr = wstrServerURL.c_str();
    wstr.append(wstrFileToDownload.begin(), wstrFileToDownload.end());
    BYTE* pByteBuffer  = NULL;
    DWORD dwBytesRead  = 0;
    DWORD dwDownloaded = 0;
    do 
    {
      // Check for available data.
      if(!WinHttpQueryDataAvailable( m_hRequest, &dwBytesRead ))
      {
          return FALSE;
      }

      // Allocate space for the buffer.
      pByteBuffer = new BYTE[dwBytesRead + 1];
      if( !pByteBuffer )
      {
        return FALSE;
      }
      else
      {
        // Read the data.
        memset(pByteBuffer, 0, dwBytesRead + 1 );

        if( !WinHttpReadData( m_hRequest, (LPVOID)pByteBuffer, dwBytesRead, &dwDownloaded ) )
        {
            return FALSE;
        }
        lCurrFileProgress += dwBytesRead;
        //printf("%s", pByteBuffer);
        
        objFileHelper.WriteBuffer(pByteBuffer, dwBytesRead);
        if((TRUE == bDisplayProgress) && (0 != lFileSize))
        {
            DisplayProgress(lCurrFileProgress, lFileSize);
        }

        // Free the memory allocated to the buffer.
        delete [] pByteBuffer;
      }
    } while( dwBytesRead > 0 );
    if((TRUE == bDisplayProgress) && (0 != lFileSize))
    {
        printf("\rDownloading: [ DONE ]");
        for(int i = 0; i < DOWNLOAD_PROGRESS_BARS; i++)
        {
            printf(" ");
        }
    }

    // Get File Name
    if(FALSE == objFileHelper.GetFileName(wstrFileName))
    {
        hr = E_FAIL;
    }
    return S_OK;
}

void ContentDownloader::DisplayProgress(long lProgress, long lTotal)
{
    int iBars = DOWNLOAD_PROGRESS_BARS;
    int iCurrentBar = 0;
    float fPercentage = 0;
    if(0 > lProgress)
    {
        lProgress = 0;
    }
    if(0 > lTotal)
    {
        return;
    }
    printf("\rDownloading: [");
    fPercentage = (float) (lProgress * 100 ) / lTotal;
    iCurrentBar = (int) ((fPercentage / 100) * iBars);
    for(int i = 0; i < iBars; i++)
    {
        if(iCurrentBar)
        {
            printf("%c", '=');
            iCurrentBar--;
        }
        else
        {
            printf("%c", ' ');
        }
    }
    printf("]");
}

void ContentDownloader::CloseAllHandles()
{
    if(NULL != m_hRequest)
    {
        WinHttpCloseHandle(m_hRequest);
    }
    if(NULL != m_hSession)
    {
        WinHttpCloseHandle(m_hSession);
    }
    if(NULL != m_hConnect)
    {
        WinHttpCloseHandle(m_hConnect);
    }
    if(NULL != m_hInternet)
    {
        WinHttpCloseHandle(m_hInternet);
    }
}