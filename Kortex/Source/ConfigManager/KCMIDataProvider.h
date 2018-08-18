#pragma once
#include "stdafx.h"

class KCMConfigEntryBase;
class KCMConfigEntryPath;
class KCMConfigEntryStd;
class KCMFileEntry;
class KCMIDataProvider
{
	public:
		KCMIDataProvider();
		virtual ~KCMIDataProvider();

	public:
		virtual bool IsOK() const = 0;

		virtual void Save() = 0;
		virtual void Load() = 0;
		virtual void UnLoad() = 0;

		virtual void OnLoadUnknownEntries(KCMFileEntry* fileEntry) = 0;

		virtual void ProcessLoadEntry(KCMConfigEntryStd* configEntry) = 0;
		virtual void ProcessSaveEntry(KCMConfigEntryStd* configEntry) = 0;
		virtual void ProcessRemoveEntry(KCMConfigEntryStd* configEntry) = 0;
		virtual void ProcessRemovePath(KCMConfigEntryPath* configEntry) = 0;
};
typedef std::vector<KCMIDataProvider*> KCMIDataProviderArray;
