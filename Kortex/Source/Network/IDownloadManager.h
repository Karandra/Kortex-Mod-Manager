#pragma once
#include "stdafx.h"
#include "GameInstance/GameID.h"
#include "IDownloadEntry.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IModSource;
	class IModRepository;
}

namespace Kortex
{
	namespace DownloadManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IDownloadManager:
		public ManagerWithTypeInfo<IPluggableManager, DownloadManager::Internal::TypeInfo>,
		public KxSingletonPtr<IDownloadManager>,
		public RTTI::IInterface<IDownloadManager>
	{
		protected:
			static wxString RenameIncrement(const wxString& name);

		public:
			IDownloadManager();
			virtual ~IDownloadManager();

		public:
			virtual void OnChangeEntry(const IDownloadEntry& entry, bool noSave = false) const = 0;
			virtual void OnAddEntry(const IDownloadEntry& entry) const = 0;
			virtual void OnRemoveEntry(const IDownloadEntry& entry) const = 0;

			virtual void OnDownloadComplete(IDownloadEntry& entry) = 0;
			virtual void OnDownloadPaused(IDownloadEntry& entry) = 0;
			virtual void OnDownloadStopped(IDownloadEntry& entry) = 0;
			virtual void OnDownloadResumed(IDownloadEntry& entry) = 0;
			virtual void OnDownloadFailed(IDownloadEntry& entry) = 0;

		public:
			virtual void LoadDownloads() = 0;
			virtual void SaveDownloads() = 0;
			void PauseAllActive();

			virtual bool ShouldShowHiddenDownloads() const = 0;
			virtual void ShowHiddenDownloads(bool show = true) = 0;

			virtual wxString GetDownloadsLocation() const = 0;
			virtual void SetDownloadsLocation(const wxString& location) = 0;

			virtual const IDownloadEntry::Vector& GetDownloads() const = 0;
			virtual IDownloadEntry::Vector& GetDownloads() = 0;
			IDownloadEntry::RefVector GetNotRunningDownloads(bool installedOnly = false) const;
		
			IDownloadEntry* FindDownloadByFileName(const wxString& name, const IDownloadEntry* except = nullptr) const;
			template<class T> static auto GetDownloadIterator(T& items, const IDownloadEntry& entry)
			{
				return std::find_if(items.begin(), items.end(), [&entry](const auto& v)
				{
					return v.get() == &entry;
				});
			}
			void AutoRenameIncrement(IDownloadEntry& entry) const;

			virtual IDownloadEntry& NewDownload() = 0;
			virtual bool RemoveDownload(IDownloadEntry& download) = 0;
			virtual bool QueueDownload(const ModDownloadReply& downloadInfo,
									   const ModFileReply& fileInfo,
									   IModRepository& modRepository,
									   const GameID& id = {}
			) = 0;
			virtual bool QueueFromOutside(const wxString& link) = 0;
	};

	class IDownloadManagerNXM: public RTTI::IInterface<IDownloadManagerNXM>
	{
		public:
			static bool CheckCmdLineArgs(const wxCmdLineParser& args, wxString& link);

		public:
			virtual bool IsAssociatedWithNXM() const = 0;
			virtual void AssociateWithNXM() = 0;

			virtual bool QueueNXM(const wxString& link) = 0;
	};
}
