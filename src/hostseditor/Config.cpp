#include "StdAfx.h"
#include "Config.h"

#include "Util.h"


CConfig::CConfig(void)
{
    TCHAR szCfgPath[MAX_PATH];
    ::GetModuleFileName(NULL, szCfgPath, MAX_PATH);
    _tcsncat(szCfgPath, _T(".cfg"), MAX_PATH);
    m_strConfigPath = szCfgPath;
}

CConfig::~CConfig(void)
{
	Clear();
}

CConfig& CConfig::instance()
{
    static CConfig inst;
    return inst;
}

BOOL CConfig::Load()
{
	Clear();

	LPBYTE pData = NULL;
	DWORD dwLength = 0;

	if(!Util::ReadFile(m_strConfigPath, pData, dwLength))
		return FALSE;

	if(pData == NULL || dwLength == 0)
		return TRUE;

    LPBYTE pTrueData = (dwLength >= 2 && pData[0] == 0xFF && pData[1] == 0xFE)
        ? pData + 2 : pData;
	BOOL bResult = ParseData(pTrueData, dwLength);
	free(pData);
	return bResult;
}

BOOL CConfig::Save()
{
	LPBYTE pData = NULL;
	DWORD dwLength = 0;
	if(!PrepareData(pData, dwLength))
		return FALSE;

	BOOL bResult = Util::WriteFile(m_strConfigPath, pData, dwLength);
	free(pData);

	return bResult;
}

BOOL CConfig::IsNameExists(LPCTSTR szName) const
{
	POSITION posMode = m_HostsModes.GetHeadPosition();
	while(posMode != NULL)
	{
		const stModeData& mode = m_HostsModes.GetNext(posMode);
        if(mode.strName.CompareNoCase(szName) == 0)
            return TRUE;
	}
    return FALSE;
}

void CConfig::AddMode(const stModeData& data)
{
    m_HostsModes.AddTail(data);
}

BOOL CConfig::IsValidModeName(LPCTSTR szName)
{
    if(szName == NULL || szName[0] == 0)
        return FALSE;

    CString strName(szName);
    DWORD dwLength = strName.GetLength();
    for(DWORD i=0; i<dwLength; ++ i)
    {
        TCHAR ch = strName[i];
        if((ch >= _T('A') && ch <= _T('Z'))
            || (ch >= _T('a') && ch <= _T('z'))
            || (ch >= _T('0') && ch <= _T('9'))
            || (ch == _T('.')))
        {
            continue;
        }
        else
        {
            return FALSE;
        }
    }
    return TRUE;
}

HostsModes& CConfig::GetHostsModes()
{
	return m_HostsModes;
}

BOOL CConfig::IsConfigExists() const
{
    return Util::IsFileExists(m_strConfigPath);
}

BOOL CConfig::RenameMode(DWORD dwModeId, LPCTSTR szNewName)
{
    stModeData* pData = GetModeById(dwModeId);
    if(pData == NULL)
        return FALSE;

    pData->strName = szNewName;
    return TRUE;
}

BOOL CConfig::RemoveById(DWORD dwModeId)
{
	POSITION posMode = m_HostsModes.GetHeadPosition();
	while(posMode != NULL)
	{
        POSITION curPos = posMode;
		stModeData& mode = m_HostsModes.GetNext(posMode);
        if(mode.dwModeId == dwModeId)
        {
            m_HostsModes.RemoveAt(curPos);
            return TRUE;
        }
	}
    return FALSE;
}

stModeData* CConfig::GetModeById(DWORD dwModeId)
{
	POSITION posMode = m_HostsModes.GetHeadPosition();
	while(posMode != NULL)
	{
		stModeData& mode = m_HostsModes.GetNext(posMode);
        if(mode.dwModeId == dwModeId)
        {
            return &mode;
        }
	}
    return NULL;
}

void CConfig::Clear()
{
	m_HostsModes.RemoveAll();
}

BOOL CConfig::ParseData(LPCVOID pData, DWORD dwLength)
{
	CString strData(static_cast<LPCTSTR>(pData), dwLength / sizeof(TCHAR));

	int nStart = 0;
	CString strLine;
	
	stModeData mode;
	while( (strLine = strData.Tokenize(_T("\n"), nStart)) && nStart != -1)
	{
		DWORD dwLineLength = strLine.GetLength();
		strLine.Trim(_T("\r\n \t"));
		dwLineLength = strLine.GetLength();
		if(strLine.GetLength() == 0)
			continue;

        if(strLine[0] == _T('['))
		{
            if(mode.IsValid())
            {
				m_HostsModes.AddTail(mode);
                mode.Empty();
            }

            int pos = strLine.Find(_T('|'));
            if(pos > 0)
            {
                mode.dwModeId = _ttoi((LPCTSTR)strLine + pos + 1);
                if(mode.dwModeId > 0)
    			    mode.strName = strLine.Mid(1, pos - 1);
            }
		}
        else if(mode.strName.GetLength() > 0)
		{
			mode.strContent += strLine;
            mode.strContent += _T("\r\n");
		}
	}
	
    if(mode.IsValid())
	{
		m_HostsModes.AddTail(mode);
	}
	return TRUE;
}

BOOL CConfig::PrepareData(LPBYTE& pData, DWORD& dwLength)
{
	CString strData;

    CString strTemp;
	POSITION posMode = m_HostsModes.GetHeadPosition();
	while(posMode != NULL)
	{
		stModeData& mode = m_HostsModes.GetNext(posMode);

        strTemp.Format(_T("%u"), mode.dwModeId);

		strData += _T("[");
		strData += mode.strName;
        strData += _T("|");
		strData += strTemp;
		strData += _T("]\r\n");

		strData += mode.strContent;
		strData += _T("\r\n");
	}

	dwLength = strData.GetLength() * sizeof(TCHAR);
	pData = static_cast<LPBYTE>(malloc(dwLength));
	if(pData == NULL)
		return FALSE;

	memcpy(pData, (LPCTSTR)strData, dwLength);
	return TRUE;
}

