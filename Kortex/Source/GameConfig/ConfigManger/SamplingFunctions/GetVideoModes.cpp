#include "stdafx.h"
#include "GetVideoModes.h"
#include "Utility/KAux.h"
#include <KxFramework/KxIndexedEnum.h>
#include <KxFramework/KxSystemSettings.h>

namespace
{
	using Component = Kortex::GameConfig::SamplingFunction::GetVideoModes::Component;

	class ComponentDef: public KxIndexedEnum::Definition<ComponentDef, Component, wxString, true>
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
	void GetVideoModes::DoCall(Component component)
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
					std::tuple<int64_t, int64_t> value(videoMode.Width, videoMode.Height);
					SampleValue* sample = AddValueIfUnique(videoMode.Width ^ videoMode.Height, value);
					if (sample)
					{
						auto[x, y] = value;
						sample->SetLabel(KAux::GetResolutionRatio(wxSize(x, y)));
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
				DoCall(component);
			}
		}
	}
}
