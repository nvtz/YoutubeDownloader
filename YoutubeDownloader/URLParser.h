/***********************************************************
 *
 * URLParser.h
 *
 *         Responsible for download the contents
 *
 ***********************************************************/
#include "stdafx.h"
#include "FileHelper.h"
#include "ContentDownloader.h"

#ifndef URLPARSER_H
#define URLPARSER_H

// MACROS
#define FOUR_KB (4 * 1024)
#define PRE_VIDEO_ID_URL_STRING  L"http://www.youtube.com/get_video_info?&video_id="
#define POST_VIDEO_ID_URL_STRING L"&el=detailpage&ps=default&eurl=&gl=US&hl=en"

class URLParser
{
public:
    enum VIDEO_URL_PARSER
    {
        YOUTUBE_VIDEO_URL_PARSER,
    };
public:
    URLParser();
    ~URLParser();
    HRESULT Parse(std::wstring& wstrURL, std::vector<KEY_VAL_PAIR>& v_pairKeyVal);
private:
    HRESULT ParseVideoURL(std::wstring& wstrURL, std::wstring& wstrVideoURL);
    HRESULT GetVideoInfoURL(URLParser::VIDEO_URL_PARSER eVideoURLParser, std::wstring& wstrURL, std::wstring& wstrVideoURL);
    HRESULT PopulateMetaData(std::vector<KEY_VAL_PAIR>& v_pairKeyVal);
    BOOL FindTitle(std::vector<KEY_VAL_PAIR>& v_pairKeyVal);
    BOOL FindVideos(std::vector<KEY_VAL_PAIR>& v_pairKeyVal);
    int  GetVideoFormat(std::string& strRawString);
public:
    static WCHAR* StringToUnicode(std::string& strText);
    static BOOL   Replace(std::string& strSource, std::string& strToFind, std::string& ptrToReplace);
private:
    ContentDownloader m_objContentDownloader;
    FileHelper        m_objFileHelper;
};

#endif