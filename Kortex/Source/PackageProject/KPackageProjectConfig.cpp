#include "stdafx.h"
#include "KPackageProjectConfig.h"
#include "KPackageProject.h"
#include "KApp.h"
#include "KAux.h"

const wxString KPackageProjectConfig::ms_DefaultCompressionMethod = "LZMA2";

bool KPackageProjectConfig::IsCompressionMethodSupported(const wxString& value)
{
	return value == "LZMA" || value == "LZMA2" || value == "BZip2" || value == "PPMd";
}

KPackageProjectConfig::KPackageProjectConfig(KPackageProject& project)
	:KPackageProjectPart(project)
{
}
KPackageProjectConfig::~KPackageProjectConfig()
{
}

void KPackageProjectConfig::SetCompressionMethod(const wxString& value)
{
	if (IsCompressionMethodSupported(value))
	{
		m_CompressionMethod = value;
	}
	else
	{
		m_CompressionMethod = ms_DefaultCompressionMethod;
	}
}
void KPackageProjectConfig::SetCompressionLevel(int value)
{
	if (value >= ms_MinCompressionLevel && value <= ms_MaxCompressionLevel)
	{
		m_CompressionLevel = value;
	}
	else
	{
		m_CompressionLevel = ms_DefaultCompressionLevel;
	}
}
void KPackageProjectConfig::SetCompressionDictionarySize(int value)
{
	if (value >= ms_MinDictionarySize && value <= ms_MaxDictionarySize)
	{
		m_CompressionDictionarySize = value;
	}
	else
	{
		m_CompressionDictionarySize = ms_DefaultDictionarySize;
	}
}
