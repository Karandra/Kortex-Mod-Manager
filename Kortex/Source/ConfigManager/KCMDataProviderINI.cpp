#include "stdafx.h"
#include "KConfigManager.h"
#include "KCMDataProviderINI.h"
#include "KCMConfigEntry.h"
#include "KCMFileEntry.h"
#include "GameInstance/KGameInstance.h"
#include <KxFramework/KxFileStream.h>


void KCMDataProviderINIBase::OnLoadUnknownEntries(KCMFileEntry* fileEntry)
{
	KCMValueTypeDetector* detector = fileEntry->GetConfigManager()->OnQueryTypeDetector(fileEntry->GetProfileEntry()->GetTypeDetectorID());

	KxStringVector tSectionNames = GetDocument().GetSectionNames();
	for (const wxString& sectionName: tSectionNames)
	{
		if (!sectionName.IsEmpty())
		{
			KCMConfigEntryPath* pathEntry = new KCMConfigEntryPath(fileEntry, sectionName);
			pathEntry->SetUnknownEntry(true);
			fileEntry->AddEntry(pathEntry);

			KxStringVector tKeyNames = GetDocument().GetKeyNames(sectionName);
			for (const wxString& keyName: tKeyNames)
			{
				if (!keyName.IsEmpty())
				{
					KCMDataType type = KCMDT_UNKNOWN;
					if (detector)
					{
						type = (*detector)(keyName, GetDocument().GetValue(sectionName, keyName));
					}

					KCMConfigEntryStd* stdEntry = new KCMConfigEntryStd(fileEntry, fileEntry->GetFormatter());
					stdEntry->SetUnknownEntry(true);
					stdEntry->Create(sectionName, keyName, type);
					fileEntry->AddEntry(stdEntry);
				}
			}
		}
	}
}

void KCMDataProviderINIBase::ProcessLoadEntry(KCMConfigEntryStd* configEntry)
{
	configEntry->LoadEntry(GetDocument());
}
void KCMDataProviderINIBase::ProcessSaveEntry(KCMConfigEntryStd* configEntry)
{
	configEntry->SaveEntry(GetDocument());
}
void KCMDataProviderINIBase::ProcessRemoveEntry(KCMConfigEntryStd* configEntry)
{
	configEntry->RemoveEntry(GetDocument());
}
void KCMDataProviderINIBase::ProcessRemovePath(KCMConfigEntryPath* configEntry)
{
	configEntry->RemovePath(GetDocument());
}

//////////////////////////////////////////////////////////////////////////
void KCMDataProviderINI::Init(const wxString& documentPath)
{
	m_DocumentPath = documentPath;
}
KCMDataProviderINI::KCMDataProviderINI()
{
}
KCMDataProviderINI::KCMDataProviderINI(const wxString& documentPath)
	:m_DocumentPath(documentPath)
{
}
KCMDataProviderINI::~KCMDataProviderINI()
{
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
