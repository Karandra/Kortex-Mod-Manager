#pragma once
#include "Framework.hpp"

namespace Kortex
{
	class IGameInstance;
}

namespace Kortex
{
	class GameID final
	{
		private:
			kxf::String m_ID;

		public:
			GameID(const kxf::String& id = {})
				:m_ID(id)
			{
			}
			GameID(const IGameInstance& instance);
			GameID(const GameID&) = default;
			GameID(GameID&&) noexcept = default;

		public:
			bool IsNull() const noexcept
			{
				return m_ID.IsEmpty();
			}

			kxf::String ToString() const
			{
				return m_ID;
			}
			operator kxf::String() const
			{
				return m_ID;
			}

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}

			GameID& operator=(const GameID&) = default;
			GameID& operator=(GameID&&) noexcept = default;

			bool operator==(const GameID& other) const noexcept
			{
				return m_ID == other.m_ID;
			}
			bool operator==(const kxf::String& other) const noexcept
			{
				return m_ID == other;
			}
			bool operator!=(const GameID& other) const noexcept
			{
				return m_ID != other.m_ID;
			}
			bool operator!=(const kxf::String& other) const noexcept
			{
				return m_ID != other;
			}
	};
}

namespace Kortex::GameIDs
{
	namespace TheElderScrolls
	{
		extern const GameID Morrowind;
		extern const GameID Oblivion;
		extern const GameID Skyrim;
		extern const GameID SkyrimSE;
		extern const GameID SkyrimVR;
	}
	namespace Fallout
	{
		extern const GameID Fallout3;
		extern const GameID FalloutNV;
		extern const GameID Fallout4;
		extern const GameID Fallout4VR;
	}
	
	extern const GameID Sacred2;
};
