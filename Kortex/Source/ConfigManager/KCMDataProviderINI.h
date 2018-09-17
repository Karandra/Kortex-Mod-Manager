#pragma once
#include "stdafx.h"
#include "KCMIDataProvider.h"
#include <KxFramework/KxINI.h>

class KCMConfigEntryStd;
class KCMFileEntry;
class KCMDataProviderINI: public KCMIDataProvider
{
	private:
		wxString m_DocumentPath;
		KxINI m_Document;

	public:
		void Init(const wxString& sDocumentPath);
		KCMDataProviderINI();
		KCMDataProviderINI(const wxString& sDocumentPath);
		virtual ~KCMDataProviderINI();

	public:
		KxINI& GetDocument()
		{
			return m_Document;
		}
		const KxINI& GetDocument() const
		{
			return m_Document;
		}
		const wxString& GetDocumentPath() const
		{
			return m_DocumentPath;
		}

	public:
		virtual bool IsOK() const override;

		virtual void Save() const override;
		virtual void Load() override;
		virtual void UnLoad() override;

		virtual void OnLoadUnknownEntries(KCMFileEntry* fileEntry) override;

		virtual void ProcessLoadEntry(KCMConfigEntryStd* configEntry) override;
		virtual void ProcessSaveEntry(KCMConfigEntryStd* configEntry) override;
		virtual void ProcessRemoveEntry(KCMConfigEntryStd* configEntry) override;
		virtual void ProcessRemovePath(KCMConfigEntryPath* configEntry) override;
};
