#pragma once
#include "Framework.hpp"

namespace Kortex
{
	class IGameDefinition;
}

namespace Kortex
{
	class GameID final
	{
		private:
			kxf::String m_ID;

		public:
			GameID(kxf::String id = {})
				:m_ID(std::move(id))
			{
			}
			GameID(const IGameDefinition& definition);
			GameID(const GameID&) = default;
			GameID(GameID&&) noexcept = default;

		public:
			bool IsNull() const noexcept
			{
				return m_ID.IsEmpty();
			}

			const kxf::String& ToString() const& noexcept
			{
				return m_ID;
			}
			kxf::String ToString() && noexcept
			{
				return std::move(m_ID);
			}

			operator const kxf::String&() const& noexcept
			{
				return m_ID;
			}
			operator kxf::String() && noexcept
			{
				return std::move(m_ID);
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

namespace std
{
	template<>
	struct hash<Kortex::GameID>
	{
		size_t operator()(const Kortex::GameID& id) const noexcept
		{
			return std::hash<kxf::String>()(id.ToString());
		}
	};
}
