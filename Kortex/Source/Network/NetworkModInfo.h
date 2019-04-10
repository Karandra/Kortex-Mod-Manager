#pragma once
#include "Common.h"

namespace Kortex
{
	class NetworkModInfo
	{
		public:
			static wxTextValidator GetValidator();

		private:
			ModID m_ModID;
			ModFileID m_FileID;

		public:
			NetworkModInfo() = default;
			NetworkModInfo(ModID id)
				:m_ModID(id)
			{
			}
			NetworkModInfo(ModFileID id)
				:m_FileID(id)
			{
			}
			NetworkModInfo(ModID modID, ModFileID fileID)
				:m_ModID(modID), m_FileID(fileID)
			{
			}
			
		public:
			bool IsEmpty() const
			{
				return !HasModID() && !HasFileID();
			}
			
			wxString ToString() const
			{
				if (HasFileID())
				{
					return KxString::Format(wxS("%1:%2"), m_ModID.GetValue(), m_FileID.GetValue());
				}
				return m_ModID.ToString();
			}
			bool FromString(const wxString& stringValue);

			bool HasModID() const
			{
				return m_ModID.HasValue();
			}
			ModID GetModID() const
			{
				return m_ModID;
			}

			bool HasFileID() const
			{
				return m_FileID.HasValue();
			}
			ModFileID GetFileID() const
			{
				return m_FileID;
			}
			
		public:
			bool operator==(const NetworkModInfo& other) const
			{
				return m_ModID == other.m_ModID && m_FileID == other.m_FileID;
			}
			bool operator!=(const NetworkModInfo& other) const
			{
				return !(*this == other);
			}
	};
}
