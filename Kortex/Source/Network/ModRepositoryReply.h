#pragma once
#include "stdafx.h"
#include "Common.h"
#include "Application/RTTI.h"
#include <KxFramework/KxVersion.h>

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
}

namespace Kortex
{
	enum class ModFileCategory
	{
		Unknown = 0,
		Main,
		Optional
	};
}

namespace Kortex
{
	struct ModFileReply
	{
		public:
			ModID ModID;
			ModFileID ID;
			int64_t Size = -1;
			wxString Name;
			wxString DisplayName;
			wxString ChangeLog;
			KxVersion Version;
			wxDateTime UploadDate;
			ModFileCategory Category = ModFileCategory::Unknown;
			bool IsPrimary = false;

		public:
			bool IsOK() const
			{
				return ModID.HasValue() && ID.HasValue() && Size > 0 && !Name.IsEmpty();
			}
	};
	struct ModInfoReply
	{
		public:
			ModID ID;

			wxString Name;
			wxString Summary;
			wxString Description;
			KxVersion Version;
			wxString MainImage;

			wxString Author;
			wxString Uploader;
			wxString UploaderProfile;

			wxDateTime UploadDate;
			wxDateTime LastUpdateDate;

			bool ContainsAdultContent = false;
			ModEndorsement EndorsementState = ModEndorsement::Undecided();

			ModFileReply PrimaryFile;

		public:
			bool IsOK() const
			{
				return ID.HasValue() && !Name.IsEmpty();
			}
	};
	struct ModDownloadReply
	{
		public:
			wxString Name;
			wxString ShortName;
			wxString URL;
			
		public:
			bool IsOK() const
			{
				return !URL.IsEmpty();
			}
	};
	struct ModEndorsementReply
	{
		ModEndorsement Endorsement = ModEndorsement::Undecided();
		wxString Message;
	};
}
