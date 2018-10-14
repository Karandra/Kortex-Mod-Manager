#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;
class KSaveManager;
class KSaveFile;

class KSaveManagerConfig: public KxSingletonPtr<KSaveManagerConfig>
{
	private:
		const wxString m_SaveFileFormat;
		KSaveManager* m_Manager = NULL;
		wxString m_Location;
		KLabeledValueArray m_FileFilters;

		wxString m_PrimarySaveExt;
		wxString m_SecondarySaveExt;

	public:
		KSaveManagerConfig(KGameInstance& profile, const KxXMLNode& node);
		~KSaveManagerConfig();

	public:
		KSaveFile* QuerySaveFile(const wxString& fullPath) const;

		wxString GetSaveFileFormat() const;
		wxString GetLocation() const;
		
		bool HasFileFilter() const
		{
			return !m_FileFilters.empty();
		}
		const KLabeledValueArray& GetFileFilters() const
		{
			return m_FileFilters;
		}

		bool HasMultiFileSaveConfig() const
		{
			return !m_SecondarySaveExt.IsEmpty();
		}
		const wxString& GetPrimarySaveExtension() const
		{
			return m_PrimarySaveExt;
		}
		const wxString& GetSecondarySaveExtension() const
		{
			return m_SecondarySaveExt;
		}
};
