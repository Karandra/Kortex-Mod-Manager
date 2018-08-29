#pragma once
#include "stdafx.h"
#include "KDownloadEntry.h"
#include "KPluggableManager.h"
#include "Profile/KProfileID.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KDownloadManagerWorkspace;

class KDownloadManager: public KPluggableManager, public KxSingletonPtr<KDownloadManager>
{
	friend class KDownloadEntry;
	friend class KDownloadManagerWorkspace;

	public:
		static KProfileID TranslateNxmGameID(const wxString& id);
		static bool CheckCmdLineArgs(const wxCmdLineParser& args, wxString& link);

	private:
		KDownloadEntry::Container m_Downloads;
		KProgramOptionUI m_Options;
		bool m_IsAssociatedWithNXM = false;
		bool m_IsReady = false;

	private:
		KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;
		void SetReady(bool value = true)
		{
			m_IsReady = true;
		}

		void OnChangeEntry(const KDownloadEntry& entry, bool noSerialize = false) const;
		void OnAddEntry(const KDownloadEntry& entry) const;
		void OnRemoveEntry(const KDownloadEntry& entry) const;

		void OnDownloadComplete(KDownloadEntry& entry);
		void OnDownloadPaused(KDownloadEntry& entry);
		void OnDownloadStopped(KDownloadEntry& entry);
		void OnDownloadResumed(KDownloadEntry& entry);
		void OnDownloadFailed(KDownloadEntry& entry);

		bool CheckIsAssociatedWithNXM() const;

	public:
		KDownloadManager();
		virtual ~KDownloadManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_ARROW_270;
		}

	public:
		bool IsAssociatedWithNXM() const
		{
			return m_IsAssociatedWithNXM;
		}
		void AssociateWithNXM();

		void LoadDownloads();
		void SaveDownloads();
		void OnShutdown();
		void PauseAllActive();

		bool ShouldShowHiddenDownloads() const
		{
			return m_Options.GetAttributeBool("ShowHiddenDownloads");
		}
		void ShowHiddenDownloads(bool show = true)
		{
			m_Options.SetAttribute("ShowHiddenDownloads", show);
		}

		wxString GetDownloadsLocation() const
		{
			return m_Options.GetAttribute("DownloadsLocation");
		}
		void SetDownloadsLocation(const wxString& location)
		{
			m_Options.SetAttribute("DownloadsLocation", location);
		}

		KDownloadEntry::RefContainer GetNotRunningItems(bool installedOnly = false) const;
		const KDownloadEntry::Container& GetDownloads() const
		{
			return m_Downloads;
		}
		KDownloadEntry::Container& GetDownloads()
		{
			return m_Downloads;
		}
		auto FindEntryIter(const KDownloadEntry& entry) const
		{
			return std::find_if(m_Downloads.begin(), m_Downloads.end(), [&entry](const auto& v)
			{
				return v.get() == &entry;
			});
		}
		KDownloadEntry* FindEntryWithFileName(const wxString& name, const KDownloadEntry* except = NULL) const;
		wxString RenameIncrement(const wxString& name) const;
		void AutoRenameIncrement(KDownloadEntry& entry) const;

		bool RemoveDownload(KDownloadEntry& download);
		bool QueueDownload(const KNetworkProvider::DownloadInfo& downloadInfo,
						   const KNetworkProvider::FileInfo& fileInfo,
						   const KNetworkProvider* provider,
						   const KProfileID& id = KProfileIDs::NullProfileID
		);
		bool QueueFromOutside(const wxString& link);
		bool QueueNXM(const wxString& link);
};
