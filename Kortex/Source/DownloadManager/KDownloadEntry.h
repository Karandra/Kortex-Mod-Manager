#pragma once
#include "stdafx.h"
#include "Profile/KProfileID.h"
#include "Network/KNetworkProvider.h"
#include <KxFramework/KxISerializer.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFileStream.h>
class KQuickThread;
class KProfile;

class KDownloadEntry: public KxISerializer
{
	public:
		using Container = std::vector<std::unique_ptr<KDownloadEntry>>;
		using RefContainer = std::vector<std::reference_wrapper<KDownloadEntry>>;

	private:
		KNetworkProvider::DownloadInfo m_DownloadInfo;
		KNetworkProvider::FileInfo m_FileInfo;

		wxEvtHandler m_EvtHandler;
		KQuickThread* m_Thread = NULL;
		std::unique_ptr<KxFileStream> m_Stream;
		std::unique_ptr<KxCURLSession> m_Session;

		const KProfile* m_TargetProfile = NULL;
		const KNetworkProvider* m_Provider = NULL;
		wxDateTime m_Date;
		int64_t m_DownloadedSize = 0;
		int64_t m_Speed = 0;

		bool m_IsInstalled = false;
		bool m_IsPaused = false;
		bool m_IsHidden = false;
		bool m_IsFailed = false;

	private:
		void Create();
		void CleanupDownload();
		bool RequestNewLink();

		void OnDownload(KxCURLEvent& event);
		void OnThreadExit(wxNotifyEvent& event);
		void DoRun(int64_t resumePos = 0);

	public:
		KDownloadEntry();
		KDownloadEntry(const KNetworkProvider::DownloadInfo& downloadInfo,
					   const KNetworkProvider::FileInfo& fileInfo,
					   const KNetworkProvider* provider,
					   const KProfileID& id);

	public:
		bool IsOK() const
		{
			return m_DownloadInfo.IsOK() && m_FileInfo.IsOK() && m_TargetProfile && m_Provider;
		}
		wxString GetFullPath() const;
		wxString GetMetaFilePath() const;
		
		const KProfile* GetTargetProfile() const
		{
			return m_TargetProfile;
		}
		void SetTargetProfile(const KProfile* profile)
		{
			m_TargetProfile = profile;
		}
		void SetTargetProfile(const KProfileID& id);

		bool HasProvider() const
		{
			return m_Provider != NULL;
		}
		const KNetworkProvider* GetProvider() const
		{
			return m_Provider;
		}
		void SetProvider(const KNetworkProvider* provider)
		{
			m_Provider = provider;
		}
		
		wxDateTime GetDate() const
		{
			return m_Date;
		}
		void SetDate(const wxDateTime& date)
		{
			m_Date = date;
		}
		
		int64_t GetDownloadedSize() const
		{
			return m_DownloadedSize;
		}
		void SetDownloadedSize(int64_t size)
		{
			m_DownloadedSize = size;
		}

		int64_t GetSpeed() const
		{
			return m_Speed;
		}
		bool IsCompleted() const
		{
			return m_FileInfo.IsOK() && !IsFailed() && m_FileInfo.GetSize() == m_DownloadedSize;
		}
		bool CanRestart() const
		{
			return !IsRunning() && m_Provider;
		}

		const KNetworkProvider::FileInfo& GetFileInfo() const
		{
			return m_FileInfo;
		}
		KNetworkProvider::FileInfo& GetFileInfo()
		{
			return m_FileInfo;
		}

		const KNetworkProvider::DownloadInfo& GetDownloadInfo() const
		{
			return m_DownloadInfo;
		}
		KNetworkProvider::DownloadInfo& GetDownloadInfo()
		{
			return m_DownloadInfo;
		}

		bool IsRunning() const
		{
			return m_Thread != NULL;
		}
		bool IsPaused() const
		{
			return m_IsPaused;
		}
		bool IsFailed() const
		{
			return m_IsFailed;
		}
		void SetPaused(bool value)
		{
			m_IsPaused = value;
		}

		bool IsHidden() const
		{
			return m_IsHidden && !IsRunning();
		}
		void SetHidden(bool value)
		{
			m_IsHidden = value;
		}

		bool IsInstalled() const
		{
			return m_IsInstalled;
		}
		void SetInstalled(bool value)
		{
			m_IsInstalled = value;
		}

		void Stop();
		void Pause();
		void Resume();
		void Run(int64_t resumePos = 0);
		bool Restart();

	public:
		virtual bool Serialize(wxOutputStream& stream) const override;
		virtual bool DeSerialize(wxInputStream& stream) override;
		
		bool Serialize() const;
		bool DeSerialize(const wxString& path);
};
