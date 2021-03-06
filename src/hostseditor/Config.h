#pragma once


#include "ModeData.h"



class CConfig
{
    CConfig(void);
    CConfig(const CConfig&);
    CConfig& operator = (const CConfig&);
public:
    ~CConfig(void);

    static CConfig& instance();

    BOOL Load();
    BOOL Save();

    BOOL IsNameExists(LPCTSTR szName) const;
    void AddMode(const stModeData& data);

    static BOOL IsValidModeName(LPCTSTR szName);

    HostsModes& GetHostsModes();
    BOOL IsConfigExists() const;

    BOOL RenameMode(DWORD dwModeId, LPCTSTR szNewName);
    BOOL RemoveById(DWORD dwModeId);

    stModeData* GetModeById(DWORD dwModeId);

private:
    void Clear();

    BOOL ParseData(LPCVOID pData, DWORD dwLength);
    BOOL PrepareData(LPBYTE& pData, DWORD& dwLength);

private:
    CString     m_strConfigPath;
    HostsModes  m_HostsModes;
};

