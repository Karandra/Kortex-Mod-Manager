#pragma once
#include "stdafx.h"
#include <KxVirtualFileSystem/Utility.h>

namespace Kortex::VirtualFileSystem
{
	inline wxString ToWxString(const KxVFS::KxDynamicStringW& value)
	{
		return wxString(value.data(), value.size());
	}
	inline wxString ToWxString(const KxVFS::KxDynamicStringRefW& value)
	{
		return wxString(value.data(), value.size());
	}
	
	inline KxVFS::KxDynamicStringW ToKxDynamicString(const wxString& value)
	{
		return KxVFS::KxDynamicStringW(value.wc_str(), value.length());
	}
	inline KxVFS::KxDynamicStringRefW ToKxDynamicStringRef(const wxString& value)
	{
		return KxVFS::KxDynamicStringRefW(value.wc_str(), value.length());
	}
}
