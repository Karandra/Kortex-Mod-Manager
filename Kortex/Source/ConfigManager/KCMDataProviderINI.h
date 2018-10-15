#pragma once
#include "stdafx.h"
#include "KCMIDataProvider.h"
#include <KxFramework/KxINI.h>
class KCMConfigEntryStd;
class KCMFileEntry;

class KCMDataProviderWithIniDocument
{
	private:
		KCMIDataProvider* m_Provider = NULL;

	public:
		KCMDataProviderWithIniDocument(KCMIDataProvider* provider)
			:m_Provider(provider)
		{
		}

	public:
		virtual KxINI& GetDocument() = 0;
		virtual const KxINI& GetDocument() const = 0;

	public:
		KCMIDataProvider* GetProvider() const
		{
			return m_Provider;
		}
};

class KCMDataProviderINIBase: public KCMIDataProvider, public KCMDataProviderWithIniDocument
{
	public:
		KCMDataProviderINIBase()
			:KCMDataProviderWithIniDocument(this)
		{
		}

	public:
		virtual bool IsOK() const override
		{
			return GetDocument().IsOK();
		}

		virtual void UnLoad() override
		{
			GetDocument().Load(wxEmptyString);
		}
		virtual void OnLoadUnknownEntries(KCMFileEntry* fileEntry) override;

		virtual void ProcessLoadEntry(KCMConfigEntryStd* configEntry) override;
		virtual void ProcessSaveEntry(KCMConfigEntryStd* configEntry) override;
		virtual void ProcessRemoveEntry(KCMConfigEntryStd* configEntry) override;
		virtual void ProcessRemovePath(KCMConfigEntryPath* configEntry) override;
};

//////////////////////////////////////////////////////////////////////////
class KCMDataProviderINI: public KCMDataProviderINIBase
{
	private:
		wxString m_DocumentPath;
		KxINI m_Document;

	public:
		void Init(const wxString& documentPath);
		KCMDataProviderINI();
		KCMDataProviderINI(const wxString& documentPath);
		virtual ~KCMDataProviderINI();

	public:
		virtual KxINI& GetDocument() override
		{
			return m_Document;
		}
		virtual const KxINI& GetDocument() const override
		{
			return m_Document;
		}
		const wxString& GetDocumentPath() const
		{
			return m_DocumentPath;
		}

	public:
		virtual void Save() const override;
		virtual void Load() override;
};
