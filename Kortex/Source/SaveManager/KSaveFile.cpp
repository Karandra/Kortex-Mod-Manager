#include "stdafx.h"
#include "KSaveFile.h"
#include "KAux.h"

KSaveFile::KSaveFile(const wxString& filePath)
	:m_FileInfo(filePath)
{
	m_FileInfo.UpdateInfo();
}
KSaveFile::~KSaveFile()
{
}

const wxBitmap& KSaveFile::CreateThumbBitmap(int width, int height)
{
	m_Thumb = KAux::ScaleImageAspect(GetBitmap(), width, height);
	return m_Thumb;
}
