#include "stdafx.h"
#include "KSaveFile.h"

KSaveFile::KSaveFile(const wxString& filePath)
	:m_FileInfo(filePath)
{
	m_FileInfo.UpdateInfo();
}
KSaveFile::~KSaveFile()
{
}
