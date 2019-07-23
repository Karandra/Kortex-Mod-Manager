#include "stdafx.h"
#include "GameID.h"
#include "IGameInstance.h"
#include "Application/SystemApplication.h"

namespace Kortex
{
	bool GameID::TestGameID(const wxString& id) const
	{
		return !id.IsEmpty();
	}
	IGameInstance* GameID::GetInstanceByID(const wxString& id) const
	{
		if (TestGameID(id))
		{
			IGameInstance* active = IGameInstance::GetActive();
			if (active && active->GetGameID() == id)
			{
				return active;
			}
			return IGameInstance::GetShallowInstance(id);
		}
		return nullptr;
	}

	GameID::GameID(const wxString& id)
		:m_ID(TestGameID(id) ? id : wxEmptyString)
	{
	}
	GameID::GameID(const IGameInstance& instance)
		:m_ID(instance.IsOK() ? instance.GetGameID().m_ID : wxEmptyString)
	{
	}

	bool GameID::IsOK() const
	{
		return TestGameID(m_ID);
	}
	wxString GameID::ToString() const
	{
		return IsOK() ? m_ID : wxEmptyString;
	}
	IGameInstance* GameID::ToGameInstance() const
	{
		return GetInstanceByID(m_ID);
	}

	GameID& GameID::operator=(const wxString& id)
	{
		m_ID = TestGameID(id) ? id : wxEmptyString;
		return *this;
	}
}
