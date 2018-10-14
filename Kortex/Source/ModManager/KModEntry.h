#pragma once
#include "stdafx.h"
#include "KFileTreeNode.h"
#include "Network/KNetworkConstants.h"
#include "Utility/KLabeledValue.h"
#include "Utility/KWithBitmap.h"
#include <KxFramework/KxVersion.h>
#include "KAux.h"
class KFixedModEntry;
class KPriorityGroupEntry;
class KPackageProject;
class KModManager;
class KModManagerDispatcher;

enum KModManagerLocation;
enum KImageEnum;

enum KMETimeIndex
{
	KME_TIME_INVALID = -1,

	KME_TIME_INSTALL,
	KME_TIME_UNINSTALL,
	
	KME_TIME_MAX
};
enum
{
	KNETWORK_SITE_INVALID_MODID = -1,
};

class KModEntry: public KWithBitmap
{
	friend class KModManagerDispatcher;

	public:
		using Vector = std::vector<KModEntry*>;
		using RefVector = std::vector<KModEntry*>;
		using CRefVector = std::vector<const KModEntry*>;

		using FixedWebSitePair = std::pair<KNetworkModID, KNetworkProviderID>;
		using FixedWebSitesArray = std::array<KNetworkModID, KNETWORK_PROVIDER_ID_MAX>;

	public:
		static wxString GetSignatureFromID(const wxString& id);

		static int64_t GetWebSiteModID(const FixedWebSitesArray& array, KNetworkProviderID index);
		static bool HasWebSite(const FixedWebSitesArray& array, KNetworkProviderID index);
		static KLabeledValue GetWebSite(const FixedWebSitesArray& array, KNetworkProviderID index, const wxString& signature = wxEmptyString);
		static void SetWebSite(FixedWebSitesArray& array, KNetworkProviderID index, KNetworkModID modID);
		static bool HasTag(KxStringVector& array, const wxString& value);
		static bool ToggleTag(KxStringVector& array, const wxString& value, bool setTag);

	private:
		wxString m_Signature;
		wxString m_ID;
		wxString m_Name;
		KxVersion m_Version;
		wxString m_Author;
		KxStringVector m_Tags;
		wxString m_PriorityGroupTag;
		KLabeledValueArray m_WebSites;
		FixedWebSitesArray m_FixedWebSites = {KNETWORK_SITE_INVALID_MODID, KNETWORK_SITE_INVALID_MODID, KNETWORK_SITE_INVALID_MODID};
		wxDateTime m_Time[KME_TIME_MAX] = {wxDefaultDateTime, wxDefaultDateTime};
		wxString m_InstallPackageFile;
		bool m_IsEnabled = false;

		bool m_IsDescriptionChanged = false;
		mutable wxString m_Description;

		wxString m_LinkedModFilesPath;

		KFileTreeNode m_FileTree;

	private:
		bool IsInstalledReal() const;

	public:
		KModEntry();
		virtual ~KModEntry();

	public:
		void CreateFromID(const wxString& id);
		void CreateFromSignature(const wxString& signature);
		void CreateFromProject(const KPackageProject& config);
		void CreateAllFolders();
		bool Save();

	public:
		bool IsOK() const;

		virtual const KFixedModEntry* ToFixedEntry() const
		{
			return NULL;
		}
		virtual KFixedModEntry* ToFixedEntry()
		{
			return NULL;
		}
		
		virtual const KPriorityGroupEntry* ToPriorityGroup() const
		{
			return NULL;
		}
		virtual KPriorityGroupEntry* ToPriorityGroup()
		{
			return NULL;
		}

		const wxString& GetSignature() const
		{
			return m_Signature;
		}
		const wxString& GetID() const
		{
			return m_ID;
		}
		void SetID(const wxString& id);
		
		const wxString& GetName() const
		{
			if (!m_Name.IsEmpty())
			{
				return m_Name;
			}
			return m_ID;
		}
		wxString GetSafeName() const
		{
			return KAux::MakeSafeFileName(GetName());
		}
		void SetName(const wxString& value)
		{
			m_Name = value;
		}
		
		const KxVersion& GetVersion() const
		{
			return m_Version;
		}
		void SetVersion(const KxVersion& value)
		{
			m_Version = value;
		}
		
		const wxString& GetAuthor() const
		{
			return m_Author;
		}
		void SetAuthor(const wxString& value)
		{
			m_Author = value;
		}
		
		const wxDateTime& GetTime(KMETimeIndex index) const
		{
			if (index > KME_TIME_INVALID && index < KME_TIME_MAX)
			{
				return m_Time[index];
			}
			return wxDefaultDateTime;
		}
		void SetTime(KMETimeIndex index, const wxDateTime& value)
		{
			if (index > KME_TIME_INVALID && index < KME_TIME_MAX)
			{
				m_Time[index] = value;
			}
		}
		
		const KxStringVector& GetTags() const
		{
			return m_Tags;
		}
		KxStringVector& GetTags()
		{
			return m_Tags;
		}

		const wxString& GetPriorityGroupTag() const;
		void SetPriorityGroupTag(const wxString& value);

		bool IsInstallPackageFileExist() const;
		const wxString& GetInstallPackageFile() const
		{
			return m_InstallPackageFile;
		}
		void SetInstallPackageFile(const wxString& value)
		{
			m_InstallPackageFile = value;
		}
		
		KNetworkModID GetWebSiteModID(KNetworkProviderID index) const;
		bool HasWebSite(KNetworkProviderID index) const;
		KLabeledValue GetWebSite(KNetworkProviderID index) const;
		void SetWebSite(KNetworkProviderID index, KNetworkModID modID);
		
		const KLabeledValueArray& GetWebSites() const
		{
			return m_WebSites;
		}
		KLabeledValueArray& GetWebSites()
		{
			return m_WebSites;
		}
		const FixedWebSitesArray& GetFixedWebSites() const
		{
			return m_FixedWebSites;
		}
		FixedWebSitesArray& GetFixedWebSites()
		{
			return m_FixedWebSites;
		}

		bool IsDescriptionChanged() const
		{
			return m_IsDescriptionChanged;
		}
		const wxString& GetDescription() const;
		void SetDescription(const wxString& value);

		const KFileTreeNode& GetFileTree() const
		{
			return m_FileTree;
		}
		void ClearFileTree();
		void UpdateFileTree();

		virtual bool IsEnabled() const;
		virtual void SetEnabled(bool value);
		virtual bool IsInstalled() const;

		virtual bool IsLinkedMod() const
		{
			return !m_LinkedModFilesPath.IsEmpty();
		}
		void SetLinkedModLocation(const wxString& path)
		{
			m_LinkedModFilesPath = path;
		}
		
		virtual KImageEnum GetIcon() const;
		virtual intptr_t GetPriority() const;
		virtual intptr_t GetOrderIndex() const;
		virtual wxString GetLocation(KModManagerLocation index) const;
};
