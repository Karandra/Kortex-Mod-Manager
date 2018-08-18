#include "stdafx.h"
#include "KProfileID.h"
#include "KProfile.h"

KProfileID::KProfileID(const KProfile* profile)
	:m_ID(profile ? profile->GetID() : wxEmptyString)
{
}
