#pragma once
#include "stdafx.h"
#include "Common.h"
#include "Application/RTTI.h"
#include <KxFramework/KxVersion.h>

namespace Kortex::NetworkManager
{
	class BaseModReply: public RTTI::IInterface<BaseModReply>
	{
		private:
			bool m_ShouldTryLater = false;

		public:
			virtual ~BaseModReply() = default;

		public:
			virtual bool IsOK() const = 0;

			bool ShouldTryLater()
			{
				return m_ShouldTryLater;
			}
			void SetShouldTryLater(bool value = true)
			{
				m_ShouldTryLater = value;
			}
	};
}

namespace Kortex
{
	class ModEndorsement
	{
		public:
			static ModEndorsement Endorsed()
			{
				return ModEndorsement(State::Endorse);
			}
			static ModEndorsement Abstained()
			{
				return ModEndorsement(State::Abstain);
			}
			static ModEndorsement Undecided()
			{
				return ModEndorsement(State::Undecided);
			}

		private:
			enum class State
			{
				Undecided,
				Endorse,
				Abstain,
			};

		private:
			State m_State = State::Undecided;

		private:
			ModEndorsement(State value)
				:m_State(value)
			{
			}

		public:
			ModEndorsement(ModEndorsement&&) = default;
			ModEndorsement(const ModEndorsement&) = default;

		public:
			bool IsEndorsed() const
			{
				return m_State == State::Endorse;
			}
			bool IsAbstained() const
			{
				return m_State == State::Abstain;
			}
			bool IsUndecided() const
			{
				return m_State == State::Undecided;
			}

			bool ShouldRemindEndorse() const
			{
				return !IsEndorsed() && !IsAbstained();
			}
	
		public:
			ModEndorsement& operator=(ModEndorsement&&) = default;
			ModEndorsement& operator=(const ModEndorsement&) = default;
	};

	class IModFileInfo: public RTTI::IMultiInterface<IModFileInfo, NetworkManager::BaseModReply>
	{
		public:
			enum class CategoryID
			{
				Unknown = 0,
				Main,
				Optional
			};

		public:
			using Vector = std::vector<std::unique_ptr<IModFileInfo>>;

		public:
			virtual std::unique_ptr<IModFileInfo> Clone() const = 0;

		public:
			virtual ModID GetModID() const = 0;
			virtual void SetModID(ModID value) = 0;

			virtual ModFileID GetID() const = 0;
			virtual void SetID(ModFileID value) = 0;

			virtual int64_t GetSize() const = 0;
			virtual void SetSize(int64_t value) = 0;

			virtual bool IsPrimary() const = 0;

			virtual wxString GetName() const = 0;
			virtual void SetName(const wxString& value) = 0;

			virtual wxString GetDisplayName() const = 0;
			virtual void SetDisplayName(const wxString& value) = 0;

			virtual KxVersion GetVersion() const = 0;
			virtual void SetVersion(const KxVersion& value) = 0;

			virtual wxString GetChangeLog() const = 0;
			virtual void SetChangeLog(const wxString& value) = 0;

			virtual wxDateTime GetUploadDate() const = 0;
			virtual CategoryID GetCategory() const = 0;
	};

	class IModInfo: public RTTI::IMultiInterface<IModInfo, NetworkManager::BaseModReply>
	{
		public:
			virtual std::unique_ptr<IModInfo> Clone() const = 0;

		public:
			virtual ModID GetID() const = 0;

			virtual wxString GetName() const = 0;
			virtual wxString GetSummary() const = 0;
			virtual wxString GetDescription() const = 0;

			virtual wxString GetAuthor() const = 0;
			virtual wxString GetUploader() const = 0;
			virtual wxString GetUploaderProfile() const = 0;
			virtual wxString GetMainImage() const = 0;

			virtual KxVersion GetVersion() const = 0;
			virtual wxDateTime GetUploadDate() const = 0;
			virtual wxDateTime GetLastUpdateDate() const = 0;
			
			virtual IModFileInfo* GetPrimaryFile() = 0;
			virtual ModEndorsement GetEndorsementState() const = 0;

			virtual bool ContainsAdultContent() const = 0;
	};

	class IModDownloadInfo: public RTTI::IMultiInterface<IModDownloadInfo, NetworkManager::BaseModReply>
	{
		public:
			virtual std::unique_ptr<IModDownloadInfo> Clone() const = 0;

		public:
			using Vector = std::vector<std::unique_ptr<IModDownloadInfo>>;

		public:
			virtual wxString GetName() const = 0;
			virtual wxString GetShortName() const = 0;

			virtual wxString GetURL() const = 0;
			virtual void SetURL(const wxString& value) = 0;
	};

	class IModEndorsementInfo: public RTTI::IMultiInterface<IModEndorsementInfo, NetworkManager::BaseModReply>
	{
		public:
			virtual std::unique_ptr<IModEndorsementInfo> Clone() const = 0;

		public:
			virtual ModEndorsement GetEndorsement() const = 0;
			virtual wxString GetMessage() const = 0;
	};
}
