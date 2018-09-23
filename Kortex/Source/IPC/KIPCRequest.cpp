#include "stdafx.h"
#include "KIPCRequest.h"

namespace KIPCRequestNS
{
	const wxChar* TypeName[(size_t)Type::COUNT] =
	{
		wxS("None"),

		wxS("InitVFSService"),
		wxS("UninstallVFSService"),

		wxS("EnableVFS"),
		wxS("VFSStateChanged"),

		wxS("CreateMirrorVFS"),
		wxS("ClearMirrorVFSList"),

		wxS("CreateConvergenceVFS"),
		wxS("AddConvergenceVirtualFolder"),
		wxS("ClearConvergenceVirtualFolders"),
	};
};
