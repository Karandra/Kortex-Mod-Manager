#include "stdafx.h"

namespace Kortex::SystemApplicationInfo
{
	const constexpr wxChar GitCommitHash[] = 
		#include "LatestCommit.txt"
		;
}
