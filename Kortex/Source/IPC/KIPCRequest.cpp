#include "stdafx.h"
#include "KIPCRequest.h"

namespace KIPCRequestNS
{
	const wxChar* TypeName[] =
	{
		wxS("None"),

		wxS("InitVFSService"),
		wxS("UninstallVFSService"),

		wxS("ToggleVFS"),
		wxS("VFSStateChanged"),

		wxS("CreateMirrorVFS"),
		wxS("CreateMultiMirrorVFS"),
		wxS("ClearMirrorVFSList"),

		wxS("CreateConvergenceVFS"),
		wxS("AddConvergenceVirtualFolder"),
		wxS("ClearConvergenceVirtualFolders"),
		wxS("BuildConvergenceIndex"),

		wxS("BeginConvergenceIndex"),
		wxS("CommitConvergenceIndex"),
		wxS("AddConvergenceIndex"),
	};
};
