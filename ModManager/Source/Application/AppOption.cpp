#include "pch.hpp"
#include "AppOption.h"
#include "IApplication.h"
#include "GameDefinition/IGameProfile.h"
#include "GameDefinition/IGameInstance.h"
#include "Log.h"

namespace Kortex
{
	std::optional<kxf::String> AppOption::DoGetValue() const
	{
		return m_ConfigNode.GetValue();
	}
	bool AppOption::DoSetValue(const kxf::String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		const bool res = m_ConfigNode.SetValue(value, WriteEmpty::Never, asCDATA);
		NotifyChange();
		return res;
	}

	std::optional<kxf::String> AppOption::DoGetAttribute(const kxf::String& name) const
	{
		return m_ConfigNode.GetAttribute(name);
	}
	bool AppOption::DoSetAttribute(const kxf::String& name, const kxf::String& value, WriteEmpty writeEmpty)
	{
		const bool res = m_ConfigNode.SetAttribute(name, value, WriteEmpty::Never);
		NotifyChange();
		return res;
	}
	
	AppOption::AppOption() = default;
	AppOption::AppOption(const AppOption& other, const kxf::XMLNode& node)
	{
		*this = other;
		m_ConfigNode = node;
	}
	AppOption::AppOption(const AppOption&) noexcept = default;
	AppOption::~AppOption() = default;

	bool AppOption::IsNull() const
	{
		return m_ConfigNode.IsNull();
	}
	AppOption AppOption::QueryElement(const kxf::String& XPath) const
	{
		return AppOption(*this, m_ConfigNode.QueryElement(XPath));
	}
	AppOption AppOption::ConstructElement(const kxf::String& XPath)
	{
		return AppOption(*this, m_ConfigNode.ConstructElement(XPath));
	}

	void AppOption::NotifyChange()
	{
		IApplication& app = IApplication::GetInstance();

		if (m_Instance)
		{
			app.OnInstanceConfigChanged(*this, *m_Instance);
		}
		else if (m_Profile)
		{
			app.OnProfileConfigChanged(*this, *m_Profile);
		}
		else
		{
			app.OnGlobalConfigChanged(*this);
		}
	}

	AppOption& AppOption::operator=(const AppOption&) noexcept = default;
}
