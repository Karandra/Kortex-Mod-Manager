#include "stdafx.h"
#include "KSMSaveFile.h"
#include "KAux.h"

KSMSaveFile::KSMSaveFile(const wxString& filePath)
	:m_FileInfo(filePath)
{
	m_FileInfo.UpdateInfo();
}
KSMSaveFile::~KSMSaveFile()
{
}

const wxBitmap& KSMSaveFile::CreateThumbBitmap(int width, int height)
{
	m_Thumb = KAux::ScaleImageAspect(GetBitmap(), width, height);
	return m_Thumb;
}
