#pragma once
#include "stdafx.h"

#define KortexDefOption(name)	constexpr const wxChar name[] = wxS(#name)

namespace Kortex::Application::Options
{
	/* Generic attribute names */
	namespace Attribute
	{
		KortexDefOption(Enabled);
	}

	// General option "namespaces"
	namespace Option
	{
		KortexDefOption(Game);
		KortexDefOption(Instance);
		KortexDefOption(Profile);
		KortexDefOption(Variables);
		KortexDefOption(Language);
		KortexDefOption(Workspace);
		KortexDefOption(Provider);
		KortexDefOption(ImageViewer);
	}
	
	/* Application itself */
	namespace Application
	{
		KortexDefOption(RestartDelay);
	}
	namespace Language
	{
		KortexDefOption(Locale);
	}
	namespace Game
	{
		KortexDefOption(ID);
	}
	namespace Instance
	{
		KortexDefOption(ID);
		KortexDefOption(Location);
	}
	namespace Profile
	{
		KortexDefOption(ID);
	}

	// Some misc options related to application
	namespace ImageViewer
	{
		KortexDefOption(ColorBG);
		KortexDefOption(ColorFG);
	}

	/* Network */
	namespace Provider
	{
		KortexDefOption(Default);
	}

	/* Package manager */
	namespace PackageManager
	{
		KortexDefOption(Package);
		KortexDefOption(AutoShowInfo);
		KortexDefOption(FOMod);
	}
	namespace Package
	{
		KortexDefOption(Location);
		KortexDefOption(AutoShowInfo);
	}
	namespace FOMod
	{
		KortexDefOption(UseHTTPSForXMLScheme);
	}

	/* Program manager */
	namespace ProgramManager
	{
		KortexDefOption(ShowExpandedValues);
	}
}

#undef DefineOption
