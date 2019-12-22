#include "stdafx.h"
#include "GetVideoModes.h"
#include "GameConfig/IConfigManager.h"
#include "Utility/Common.h"
#include <KxFramework/KxIndexedEnum.h>
#include <KxFramework/KxSystemSettings.h>

namespace
{
	using Component = Kortex::GameConfig::SamplingFunction::GetVideoModes::Component;
	struct ComponentDef: public KxIndexedEnum::Definition<ComponentDef, Component, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{Component::None, wxS("Component::None")},
			{Component::Width, wxS("Component::Width")},
			{Component::Height, wxS("Component::Height")},
		};
	};
}

namespace Kortex::GameConfig::SamplingFunction
{
	void GetVideoModes::DoCall(Component component, const wxString& format)
	{
		std::unordered_set<uint32_t> addedValues;
		auto AddValueIfUnique = [this, &addedValues](uint32_t testValue, const auto& value) -> SampleValue*
		{
			auto[it, inserted] = addedValues.emplace(testValue);
			if (inserted)
			{
				return &m_Values.emplace_back(value);
			}
			return nullptr;
		};

		for (const KxSystemSettings::VideoMode& videoMode: KxSystemSettings::EnumVideoModes(wxEmptyString))
		{
			switch (component)
			{
				case Component::Width:
				{
					AddValueIfUnique(videoMode.Width, videoMode.Width);
					break;
				}
				case Component::Height:
				{
					AddValueIfUnique(videoMode.Height, videoMode.Height);
					break;
				}
				case Component::Both:
				{
					uint32_t hashValue = videoMode.Width ^ videoMode.Height;
					SampleValue* sample = AddValueIfUnique(hashValue, KxString::Format(format, videoMode.Width, videoMode.Height));
					if (sample)
					{
						sample->SetLabel(Utility::GetResolutionRatio(wxSize(videoMode.Width, videoMode.Height)));
						if (!sample->HasLabel())
						{
							sample->SetLabel(m_Manager.TranslateItemLabel(wxS("VideoMode.RatioUnknown"), wxS("SampleValue")));
						}
					}
					break;
				}
			};
		}
	}
	void GetVideoModes::OnCall(const ItemValue::Vector& arguments)
	{
		if (arguments.size() >= 1)
		{
			Component component = ComponentDef::FromOrExpression(arguments[0].As<wxString>(), Component::None);
			if (component != Component::None)
			{
				DoCall(component, arguments.size() >= 2 ? arguments[1].As<wxString>() : wxEmptyString);
			}
		}
	}
}
