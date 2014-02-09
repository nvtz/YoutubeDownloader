/***********************************************************
 *
 * URLParser.cpp
 *
 *         Responsible for download the contents
 *
 ***********************************************************/
#include "URLParser.h"

URLParser::URLParser()
{
}

URLParser::~URLParser()
{
}

HRESULT URLParser::Parse(std::wstring& wstrURL, std::vector<KEY_VAL_PAIR>& v_pairKeyVal)
{
    HRESULT hr = S_OK;
    std::wstring wstrOutFileName;
    std::wstring wstrVideoURL;
    
    hr = m_objContentDownloader.DownloadURL(wstrURL, wstrOutFileName);
    if(S_OK != hr)
    {
        return hr;
    }
    // Open URL
    m_objFileHelper.OpenFile(wstrOutFileName);
    // Find Title
    FindTitle(v_pairKeyVal);
    // Get Video URLs
    hr = ParseVideoURL(wstrURL, wstrVideoURL);
    if(S_OK != hr)
    {
        return hr;
    }
    // Download Video URL
    hr = m_objContentDownloader.DownloadURL(wstrVideoURL, wstrOutFileName);
    if(S_OK != hr)
    {
        return hr;
    }
    m_objFileHelper.OpenFile(wstrOutFileName);
    PopulateMetaData(v_pairKeyVal);
    return hr;
}

HRESULT URLParser::ParseVideoURL(std::wstring& wstrURL, std::wstring& wstrVideoURL)
{
    HRESULT hr = E_FAIL;
    std::wstring wstrNormalizedURL;
    URLParser::VIDEO_URL_PARSER eVideoURLParser;
    int iCount = 0;
    if(0 == wstrURL.find(L"http://"))
    {
        iCount += wcslen(L"http://");
    }
    if(iCount == wstrURL.find(L"www."))
    {
        iCount += wcslen(L"www.");
    }
    if(iCount == wstrURL.find(L"youtube"))
    {
        eVideoURLParser = URLParser::YOUTUBE_VIDEO_URL_PARSER;
        hr = GetVideoInfoURL(eVideoURLParser, wstrURL, wstrVideoURL );
    }
    /*
    else if(OTHER_VIDEO_PARSERS)
    {
        // For Future
    }
    */
    return hr;
}

HRESULT URLParser::GetVideoInfoURL(URLParser::VIDEO_URL_PARSER eVideoURLParser, std::wstring& wstrURL, std::wstring& wstrVideoURL)
{
    HRESULT hr          = E_FAIL;
    int     iVdoIDStart = -1;
    int     iVdoIDEnd   = -1;
    std::wstring wstrVideoID;
    if(std::wstring::npos != (iVdoIDStart = wstrURL.find(L"v=")))
    {
        iVdoIDStart += wcslen(L"v=");
        iVdoIDEnd = wstrURL.find(L"&", iVdoIDStart);
        if(std::wstring::npos != iVdoIDEnd)
        {
            // pick start to end
            wstrVideoID = wstrURL.substr(iVdoIDStart, (iVdoIDEnd - iVdoIDStart));
        }
        else
        {
            // pick the entire string
            wstrVideoID = wstrURL.substr(iVdoIDStart, (wstrURL.length() - iVdoIDStart));
        }
    }
    if(0 != wstrVideoID.length())
    {
        wstrVideoURL.clear();
        wstrVideoURL.assign(PRE_VIDEO_ID_URL_STRING);
        wstrVideoURL.append(wstrVideoID);
        wstrVideoURL.append(POST_VIDEO_ID_URL_STRING);
        hr = S_OK;
    }
    return hr;
}

HRESULT URLParser::PopulateMetaData(std::vector<KEY_VAL_PAIR>& v_pairKeyVal)
{
    HRESULT hr = E_FAIL;
    m_objFileHelper.ResetFilePosition();
    FindVideos(v_pairKeyVal);
    return hr;
}

BOOL URLParser::FindTitle(std::vector<KEY_VAL_PAIR>& v_pairKeyVal)
{
    BOOL bRetVal        = FALSE;
    WCHAR* pwch         = NULL;
    WCHAR* pwcurr       = NULL;
    int    iRawTitleLen = 0;
    std::string strTitleHeading    = "Title";
    std::string strTitleStart      = "<TITLE"; // <Title>, <Title > etc
    std::string strTitleStartEnd   = ">";
    std::string strTitleEnd        = "</"; // </Title>, </ Title> etc
    std::string strMatchAnyOfThese = "\'\"\\";
    std::string strRawTitle;
    std::string strTitle;
    KEY_VAL_PAIR pairTitle;

    if(TRUE == m_objFileHelper.FindPattern(strTitleStart, TRUE))
    {
        if(TRUE == m_objFileHelper.FindPattern(strTitleStartEnd, FALSE))
        {
            if(S_OK == m_objFileHelper.ReadBeforeTillMatchThisPattern(strTitleEnd, strRawTitle))
            {
                WCHAR* wpszTitle = StringToUnicode(strTitleHeading);
                if(NULL != wpszTitle)
                {
                    pairTitle.first.assign(wpszTitle);
                    delete[] wpszTitle;
                    wpszTitle = NULL;
                }
                wpszTitle = StringToUnicode(strRawTitle);
                if(NULL != wpszTitle)
                {
                    pwch = wpszTitle;
                    iRawTitleLen = (int)strRawTitle.length();
                    for(int i = 0; i < iRawTitleLen; i++)
                    {
                        if(!((*pwch >= L'a' && *pwch <= L'z') ||
                             (*pwch >= L'A' && *pwch <= L'Z') ||
                             (*pwch >= L'0' && *pwch <= L'9'))   )
                        {
                            (*pwch) = L'_';
                        }
                        pwch++;
                    }
                    pwch = wpszTitle;
                    pwcurr = pwch;
                    int iCurr =  iRawTitleLen;
                    // Remove extra '_'
                    while(TRUE)
                    {
                        while((*pwch == L'_') && (*pwch != 0))
                        {
                            pwch++;
                        }
                        if(*pwch == 0)
                        {
                            *pwcurr = *pwch;
                            break;
                        }
                        *pwcurr++ = *pwch++;
                        while((*pwch != L'_') && (*pwch != 0))
                        {
                            *pwcurr++ = *pwch++;
                        }
                        *pwcurr++ = *pwch++;
                    }

                    pairTitle.second.assign(wpszTitle);
                    //Clean the URL
                    delete[] wpszTitle;
                    wpszTitle = NULL;
                }
                bRetVal = TRUE;
                v_pairKeyVal.push_back(pairTitle);
            }
        }
    }

    return bRetVal;
}

BOOL URLParser::FindVideos(std::vector<KEY_VAL_PAIR>& v_pairKeyVal)
{
    BOOL bRetVal = FALSE;
    BOOL bBreak  = FALSE;
    int  iFormat = 0;
    std::string strRawFormatString;
    std::string strRawVDOHeading = "Video Format ";
    std::string strVDOHeading;
    std::string strFMT1 = "&fmt_url_map=";
    std::string strFMT2 = "fmt_url_map\":\"";
    std::string strHTTP = "http";
    std::string strPattern0 ="%25";
    std::string strPattern1 ="%2C";
    std::string strPattern2 ="%2F";
    std::string strPattern3 ="%3D";
    std::string strPattern4 ="%3F";
    std::string strPattern5 ="%3A";
    std::string strPattern6 ="%26";
    std::string strNewPattern0 ="%";
    std::string strNewPattern1 =",";
    std::string strNewPattern2 ="/";
    std::string strNewPattern3 ="=";
    std::string strNewPattern4 ="?";
    std::string strNewPattern5 =":";
    std::string strNewPattern6 ="&";
    std::string strRawVDO;
    std::string strVDO;
    KEY_VAL_PAIR pairVDO;

    if(TRUE == m_objFileHelper.FindPattern(strFMT1, FALSE, FALSE))
    {
        while(TRUE == m_objFileHelper.FindPattern(strHTTP, FALSE, FALSE))
        {
            CHAR chFormat[10] = {0}; 
            strRawFormatString.clear();
            strVDOHeading.clear();
            if(TRUE == bBreak)
            {
                break;
            }
            m_objFileHelper.MoveBackwardByNBytes(12);
            m_objFileHelper.ReadBuffer(strRawFormatString, 12);
            iFormat = GetVideoFormat(strRawFormatString);
            itoa(iFormat, chFormat, 10);
            if(S_OK == m_objFileHelper.ReadBeforeTillMatchThisPattern(strHTTP, strRawVDO))
            {
                if(0 >= strRawVDO.length())
                {
                    bRetVal = FALSE;
                }
                else
                {
                    //strRawVDO.erase(0, 5);
                    Replace(strRawVDO,strPattern0, strNewPattern0);
                    Replace(strRawVDO,strPattern1, strNewPattern1);
                    Replace(strRawVDO,strPattern2, strNewPattern2);
                    Replace(strRawVDO,strPattern3, strNewPattern3);
                    Replace(strRawVDO,strPattern4, strNewPattern4);
                    Replace(strRawVDO,strPattern5, strNewPattern5);
                    Replace(strRawVDO,strPattern6, strNewPattern6);
                    int iPercent = strRawVDO.find("%");
                    int strlen = strRawVDO.length();
                    if(iPercent != std::string::npos)
                    {
                        strRawVDO.erase(iPercent, strlen - iPercent -1);
                    }
                    int iIndex = strRawVDO.rfind(',');
                    if(strlen - 10 > iIndex)
                    {
                        iIndex = strRawVDO.rfind('&');
                        strRawVDO.erase(iIndex, strlen - 1);
                        bBreak = TRUE;
                    }
                    else if(0 > iIndex)
                    {
                        break;
                    }
                    else
                    {
                        strRawVDO.erase(iIndex, strlen - 1);
                    }
                    
                    strRawVDO = strHTTP + strRawVDO;
                    strVDOHeading.assign(strRawVDOHeading);
                    strVDOHeading.append(chFormat);
                    pairVDO.first.assign(strVDOHeading.begin(),strVDOHeading.end());
                    pairVDO.second.assign(strRawVDO.begin(), strRawVDO.end());
                    v_pairKeyVal.push_back(pairVDO);
                    strRawVDO.clear();
                    bRetVal = TRUE;
                    //m_objFileHelper.MoveBackwardByNBytes(strHTTP.length());
                }
            }
        }
    }
    else if(TRUE == m_objFileHelper.FindPattern(strFMT2, TRUE))
    {
    }
    return bRetVal;
}

int URLParser::GetVideoFormat(std::string& strRawString)
{
    int iIndex       = 0;
    int iFormat      = 0;
    int iFormatStart = 0;
    int iFormatEnd   = 0;

    std::string strFormat;
    if(0 > strRawString.length())
    {
        return -1;
    }
    iIndex = strRawString.find("%2C");
    if(std::string::npos == iIndex)
    {
        iIndex = strRawString.find("=");
        if(std::string::npos == iIndex)
        {
            return -1;
        }
        else
        {
            iFormatStart = iIndex + strlen("=");
        }
    }
    else
    {
        iFormatStart = iIndex + strlen("%2C");
    }
    iIndex = strRawString.find("%7C");
    if(std::string::npos == iIndex)
    {
        return -1;
    }
    iFormatEnd = iIndex - 1;
    if(0 > (iFormatEnd - iFormatStart))
    {
        return -1;
    }
    strFormat = strRawString.substr(iFormatStart, iFormatEnd - iFormatStart + 1);
    iFormat = atoi(strFormat.c_str());
    return iFormat;
}

WCHAR* URLParser::StringToUnicode(std::string& strText)
{
    INT iStrLen = strText.length();
    INT iTcharLen = (iStrLen * sizeof(TCHAR)) + sizeof(TCHAR);
    TCHAR* wpszName = NULL;
    try
    {
        wpszName = new TCHAR[iTcharLen];
        memset(wpszName, 0, (iTcharLen));

        MultiByteToWideChar(CP_ACP, 0, 
                            strText.c_str(), 
                            strText.length(), 
                            wpszName, 
                            iTcharLen);

    }
    catch(std::bad_alloc& e)
    {
        e.what();
        return NULL;
    }
    return wpszName;
}

BOOL URLParser::Replace(std::string& strSource, std::string& strToFind, std::string& ptrToReplace)
{
    BOOL bRetVal = FALSE;
    while(TRUE)
    {
        int iPos = strSource.find(strToFind, 0);
        if(std::string::npos == iPos)
        {
            break;
        }
        strSource.replace(iPos, strToFind.length(), ptrToReplace);
        bRetVal = TRUE;
    }
    return bRetVal;
}