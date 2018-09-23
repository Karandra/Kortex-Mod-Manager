#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include "KDataViewListModel.h"
#include "KModEntry.h"
#include "KLabeledValue.h"
#include <KxFramework/KxStdDialog.h>
class KxButton;

class KModSitesEditorNode
{
	enum Type
	{
		Fixed,
		Normal
	};

	private:
		Type m_Type;
		union Values
		{
			struct
			{
				KNetworkProviderID SiteIndex = KNETWORK_PROVIDER_ID_INVALID;
				int64_t ModID = KNETWORK_SITE_INVALID_MODID;
			} Fixed;
			KLabeledValue* Normal;

			Values(KNetworkProviderID index, int64_t fixedSiteID)
			{
				Fixed.SiteIndex = index;
				Fixed.ModID = fixedSiteID;
			}
			Values(KLabeledValue& normalValue)
				:Normal(&normalValue)
			{
			}
		} m_Values;

	public:
		KModSitesEditorNode(KNetworkProviderID index, int64_t fixedSiteID)
			:m_Type(Fixed), m_Values(index, fixedSiteID)
		{
		}
		KModSitesEditorNode(KLabeledValue& tNormalValue)
			:m_Type(Normal), m_Values(tNormalValue)
		{
		}

	public:
		bool IsFixed() const
		{
			return m_Type == Fixed;
		}
		bool IsNormal() const
		{
			return m_Type == Normal;
		}

		KNetworkProviderID GetFixedSiteIndex() const
		{
			return m_Values.Fixed.SiteIndex;
		}
		int64_t GetFixedSiteModID() const
		{
			return m_Values.Fixed.ModID;
		}
		void SetFixedSiteModID(int64_t id)
		{
			m_Values.Fixed.ModID = id;
		}
		bool HasFixedSiteModID() const
		{
			return IsFixed() && GetFixedSiteModID() != KNETWORK_SITE_INVALID_MODID;
		}
		
		KLabeledValue& GetNormalSite() const
		{
			return *m_Values.Normal;
		}

		bool operator==(const KModSitesEditorNode& other) const
		{
			if (m_Type == other.m_Type)
			{
				if (IsFixed())
				{
					return GetFixedSiteIndex() == other.GetFixedSiteIndex() && GetFixedSiteModID() == other.GetFixedSiteModID();
				}
				else if (IsNormal())
				{
					return m_Values.Normal == other.m_Values.Normal;
				}
			}
			return false;
		}
};

//////////////////////////////////////////////////////////////////////////
class KModSitesEditor: public KxDataViewListModelEx
{
	using FixedWebSitesArray = KModEntry::FixedWebSitesArray;

	private:
		std::vector<KModSitesEditorNode> m_DataVector;
		bool m_IsModified = false;

	protected:
		FixedWebSitesArray& m_FixedSites;
		KLabeledValueArray& m_Sites;

	protected:
		virtual void OnInitControl() override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

		void OnActivate(KxDataViewEvent& event);

	public:
		KModSitesEditor(KLabeledValueArray& sites, FixedWebSitesArray& fixedSites)
			:m_Sites(sites), m_FixedSites(fixedSites)
		{
		}

	public:
		virtual size_t GetItemCount() const override
		{
			return m_DataVector.size();
		}
		virtual void RefreshItems() override;
		const KModSitesEditorNode* GetLastNode() const
		{
			return m_DataVector.empty() ? NULL : &m_DataVector.back();
		}
		
		KModSitesEditorNode& AddItem(KNetworkProviderID index, int64_t fixedSiteID)
		{
			return m_DataVector.emplace_back(index, m_FixedSites[index]);
		}
		KModSitesEditorNode& AddItem(KLabeledValue& v)
		{
			return m_DataVector.emplace_back(m_Sites.emplace_back(v));
		}
		bool RemoveItem(KModSitesEditorNode& node);

		KModSitesEditorNode& GetNode(size_t row) const
		{
			return const_cast<KModSitesEditorNode&>(m_DataVector[row]);
		}
		KModSitesEditorNode& GetNode(const KxDataViewItem& item) const
		{
			return GetNode(GetRow(item));
		}

		bool IsModified() const
		{
			return m_IsModified;
		}
};

//////////////////////////////////////////////////////////////////////////
class KModSitesEditorDialog: public KxStdDialog, public KModSitesEditor
{
	private:
		wxWindow* m_ViewPane = NULL;
		KxButton* m_AddButton = NULL;
		KxButton* m_RemoveButton = NULL;

	private:
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}
		void OnSelectItem(KxDataViewEvent& event);
		void OnAddTag(wxCommandEvent& event);
		void OnRemoveTag(wxCommandEvent& event);

	public:
		KModSitesEditorDialog(wxWindow* parent, KLabeledValueArray& sites, KModEntry::FixedWebSitesArray& fixedSites);
		virtual ~KModSitesEditorDialog();
};
