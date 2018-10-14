#include "stdafx.h"
#include "KGameID.h"
#include "KGameInstance.h"

KGameID::KGameID(const KGameInstance& instance)
	:m_ID(instance.GetGameID())
{
}
KGameID::KGameID(const KGameInstance* instance)
	:m_ID(instance ? instance->GetGameID() : wxString())
{
}
