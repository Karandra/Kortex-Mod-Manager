#pragma once
#include "stdafx.h"
#include "Profile/KConfigManagerConfig.h"
#include <KxFramework/KxXDocumentNode.h>
class KxDataViewCtrl;
class KxSplitterWindow;
class KWorkspace;
class KManager;
class KInstallWizardDialog;

class KProgramOption;
class KProgramOptionSerializer
{
	friend class KProgramOption;

	private:
		static bool DoCheckSaveLoad(KProgramOption& option, bool save);
		static void DoSaveLoadDataViewLayout(KxDataViewCtrl* viewCtrl, KProgramOption& option, bool save);
		static void DoSaveLoadSplitterLayout(KxSplitterWindow* window, KProgramOption& option, bool save);
		static void DoSaveLoadWindowSize(wxTopLevelWindow* window, KProgramOption& option, bool save);

		static wxString GetValue(KPGCFileID id, const wxString& section, const wxString& name, const wxString& default = wxEmptyString);
		static void SetValue(KPGCFileID id, const wxString& section, const wxString& name, const wxString& value);

	public:
		static void SaveDataViewLayout(KxDataViewCtrl* viewCtrl, KProgramOption& option)
		{
			DoSaveLoadDataViewLayout(viewCtrl, option, true);
		}
		static void LoadDataViewLayout(KxDataViewCtrl* viewCtrl, KProgramOption& option)
		{
			DoSaveLoadDataViewLayout(viewCtrl, option, false);
		}

		static void SaveSplitterLayout(KxSplitterWindow* window, KProgramOption& option)
		{
			DoSaveLoadSplitterLayout(window, option, true);
		}
		static void LoadSplitterLayout(KxSplitterWindow* window, KProgramOption& option)
		{
			DoSaveLoadSplitterLayout(window, option, false);
		}

		static void SaveWindowSize(wxTopLevelWindow* window, KProgramOption& option)
		{
			DoSaveLoadWindowSize(window, option, true);
		}
		static void LoadWindowSize(wxTopLevelWindow* window, KProgramOption& option)
		{
			DoSaveLoadWindowSize(window, option, false);
		}
};

//////////////////////////////////////////////////////////////////////////
class KProgramOption: public KxXDocumentNode<KProgramOption>
{
	friend class KProgramOptionSerializer;

	private:
		const KPGCFileID m_FileID = KPGC_ID_INVALID;
		wxString m_SectionID;
		wxString m_NameID;

		bool m_IsLoaded = false;

	private:
		bool IsLoaded() const
		{
			return m_IsLoaded;
		}
		void MarkLoaded()
		{
			m_IsLoaded = true;
		}

	protected:
		bool IsPathsOK() const
		{
			return m_FileID != KPGC_ID_INVALID && !m_SectionID.IsEmpty();
		}

		virtual bool DoSetValue(const wxString& value, bool isCDATA = false) override;
		virtual bool DoSetAttribute(const wxString& name, const wxString& value);

		KxIntVector DoToIntVector(const wxString& values) const;
		wxString DoFromIntVector(const KxIntVector& data) const;

	public:
		KProgramOption(KPGCFileID id = KPGC_ID_INVALID, const wxString& sSectionID = wxEmptyString, const wxString& nameID = wxEmptyString)
			:m_FileID(id), m_SectionID(sSectionID), m_NameID(nameID)
		{
		}
		KProgramOption(KWorkspace* workspace, KPGCFileID id = KPGC_ID_INVALID, const wxString& nameID = wxEmptyString);
		void Init(KWorkspace* workspace, const wxString& nameID = wxEmptyString);

	public:
		/* General */
		virtual bool IsOK() const override
		{
			return IsPathsOK();
		}
		virtual Node QueryElement(const wxString& sXPath) const override
		{
			return Node();
		}

		/* Node */
		virtual size_t GetIndexWithinParent() const override
		{
			return 0;
		}
		
		wxString GetSection() const
		{
			return m_SectionID;
		}
		bool SetSection(const wxString& sSectionID)
		{
			m_SectionID = sSectionID;
			return true;
		}
		virtual wxString GetName() const override
		{
			return m_NameID;
		}
		virtual bool SetName(const wxString& name) override
		{
			m_NameID = name;
			return true;
		}

		virtual size_t GetChildrenCount() const override
		{
			return 0;
		}
		virtual NodeVector GetChildren() const override
		{
			return NodeVector();
		}

		/* Value */
		virtual wxString GetValue(const wxString& default = wxEmptyString) const override;
		KxIntVector KProgramOption::GetValueIntVector() const
		{
			return DoToIntVector(GetValue());
		}
		bool KProgramOption::SetValueIntVector(const KxIntVector& data)
		{
			return DoSetValue(DoFromIntVector(data));
		}

		/* Attributes */
		virtual size_t GetAttributeCount() const override
		{
			return 0;
		}
		virtual KxStringVector GetAttributes() const override
		{
			return KxStringVector();
		}
		virtual bool HasAttribute(const wxString& name) const override
		{
			return !GetAttribute(name).IsEmpty();
		}
		
		virtual wxString GetAttribute(const wxString& name, const wxString& default = wxEmptyString) const override;
		KxIntVector GetAttributeVectorInt(const wxString& name) const
		{
			return DoToIntVector(GetAttribute(name));
		}

		bool SetAttributeVectorInt(const wxString& name, const KxIntVector& data)
		{
			return DoSetAttribute(name, DoFromIntVector(data));
		}

		/* Navigation */
		virtual Node GetElementByAttribute(const wxString& name, const wxString& value) const override
		{
			return Node();
		}
		virtual Node GetElementByTag(const wxString& tagName) const override
		{
			return Node();
		}
		virtual Node GetParent() const override
		{
			return Node();
		}
		virtual Node GetPreviousSibling() const override
		{
			return Node();
		}
		virtual Node GetNextSibling() const override
		{
			return Node();
		}
		virtual Node GetFirstChild() const override
		{
			return Node();
		}
		virtual Node GetLastChild() const override
		{
			return Node();
		}
};

class KProgramOptionUI: public KProgramOption
{
	public:
		KProgramOptionUI(const wxString& sSectionID = wxEmptyString, const wxString& nameID = wxEmptyString)
			:KProgramOption(KPGC_ID_CURRENT_PROFILE, sSectionID, nameID)
		{
		}
		KProgramOptionUI(const wxString& nameID)
			:KProgramOption(KPGC_ID_CURRENT_PROFILE, wxEmptyString, nameID)
		{
		}

		KProgramOptionUI(KWorkspace* workspace, const wxString& nameID = wxEmptyString)
			:KProgramOption(workspace, KPGC_ID_CURRENT_PROFILE, nameID)
		{
		}
		KProgramOptionUI(KInstallWizardDialog* workspace, const wxString& nameID = wxEmptyString);
		KProgramOptionUI(KManager* manager, const wxString& nameID = wxEmptyString);
};
