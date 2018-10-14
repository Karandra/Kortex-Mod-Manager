#include "stdafx.h"
#include "KConfigManager.h"
#include "KCMDataProviderINI.h"
#include "KCMConfigEntry.h"
#include "KCMFileEntry.h"
#include "GameInstance/KGameInstance.h"
#include <KxFramework/KxFileStream.h>

//////////////////////////////////////////////////////////////////////////

void KCMDataProviderINI::Init(const wxString& sDocumentPath)
{
	m_DocumentPath = sDocumentPath;
}
KCMDataProviderINI::KCMDataProviderINI()
{
}
KCMDataProviderINI::KCMDataProviderINI(const wxString& sDocumentPath)
	:m_DocumentPath(sDocumentPath)
{
}
KCMDataProviderINI::~KCMDataProviderINI()
{
}

bool KCMDataProviderINI::IsOK() const
{
	return m_Document.IsOK();
}

void KCMDataProviderINI::Save() const
{
	KxFileStream file(m_DocumentPath, KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS);
	m_Document.Save(file);
}
void KCMDataProviderINI::Load()
{
	KxFileStream file(m_DocumentPath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
	m_Document.Load(file);
}
void KCMDataProviderINI::UnLoad()
{
	m_Document.Load(wxEmptyString);
}

void KCMDataProviderINI::OnLoadUnknownEntries(KCMFileEntry* fileEntry)
{
	KCMValueTypeDetector* pDetector = fileEntry->GetConfigManager()->OnQueryTypeDetector(fileEntry->GetProfileEntry()->GetTypeDetectorID());

	KxStringVector tSectionNames = m_Document.GetSectionNames();
	for (const wxString& sSectionName: tSectionNames)
	{
		if (!sSectionName.IsEmpty())
		{
			KCMConfigEntryPath* pathEntry = new KCMConfigEntryPath(fileEntry, sSectionName);
			pathEntry->SetUnknownEntry(true);
			fileEntry->AddEntry(pathEntry);

			KxStringVector tKeyNames = m_Document.GetKeyNames(sSectionName);
			for (const wxString& sKeyName: tKeyNames)
			{
				if (!sKeyName.IsEmpty())
				{
					KCMDataType type = KCMDT_UNKNOWN;
					if (pDetector)
					{
						type = (*pDetector)(sKeyName, m_Document.GetValue(sSectionName, sKeyName));
					}

					KCMConfigEntryStd* stdEntry = new KCMConfigEntryStd(fileEntry, fileEntry->GetFormatter());
					stdEntry->SetUnknownEntry(true);
					stdEntry->Create(sSectionName, sKeyName, type);
					fileEntry->AddEntry(stdEntry);
				}
			}
		}
	}
}

void KCMDataProviderINI::ProcessLoadEntry(KCMConfigEntryStd* configEntry)
{
	configEntry->LoadEntry(m_Document);
}
void KCMDataProviderINI::ProcessSaveEntry(KCMConfigEntryStd* configEntry)
{
	configEntry->SaveEntry(m_Document);
}
void KCMDataProviderINI::ProcessRemoveEntry(KCMConfigEntryStd* configEntry)
{
	configEntry->RemoveEntry(m_Document);
}
void KCMDataProviderINI::ProcessRemovePath(KCMConfigEntryPath* configEntry)
{
	configEntry->RemovePath(m_Document);
}
