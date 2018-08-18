#include "stdafx.h"
#include "KCMOptionsFormatter.h"
#include "KCMConfigEntry.h"
#include "KConfigManager.h"
#include "KAux.h"

KCMOptionsFormatter KCMOptionsFormatter::LoadFormatOptions(KxXMLNode& node, const KCMOptionsFormatter& tDefaultOptions)
{
	KxXMLNode tOptionsNode = node.GetFirstChildElement("FormatOptions");
	if (tOptionsNode.IsOK())
	{
		return KCMOptionsFormatter(tOptionsNode);
	}
	else
	{
		return tDefaultOptions;
	}
}

KCMOptionsFormatter::KCMOptionsFormatter()
{
}
KCMOptionsFormatter::KCMOptionsFormatter(KxXMLNode& node)
{
	Load(node);
}
void KCMOptionsFormatter::Load(KxXMLNode& node)
{
	auto AddOption = [&node](const wxString& name, const wxString& default = wxEmptyString)
	{
		KxXMLNode tOptionNode = node.GetFirstChildElement(name);
		wxString value = tOptionNode.GetValue();
		if (!value.IsEmpty())
		{
			return value;
		}
		else
		{
			return default;
		}
	};

	m_OptionInt = AddOption("int");
	m_OptionFloat = AddOption("float");
	m_OptionBool = AddOption("bool", "i");
	m_OptionString = AddOption("string");

	m_Pattern = node.GetFirstChildElement("Pattern").GetValue();
}

wxString KCMOptionsFormatter::GetFormatOption(KCMDataType type) const
{
	auto FormatOption = [this, type]() -> wxString
	{
		switch (type)
		{
			case KCMDT_INT8:
			{
				return '%' + m_OptionInt + "hhi";
			}
			case KCMDT_UINT8:
			{
				return '%' + m_OptionInt + "hhu";
			}
			case KCMDT_INT16:
			{
				return '%' + m_OptionInt + "hi";
			}
			case KCMDT_UINT16:
			{
				return '%' + m_OptionInt + "hu";
			}
			case KCMDT_INT32:
			{
				return '%' + m_OptionInt + "li";
			}
			case KCMDT_UINT32:
			{
				return '%' + m_OptionInt + "lu";
			}
			case KCMDT_INT64:
			{
				return '%' + m_OptionInt + "lli";
			}
			case KCMDT_UINT64:
			{
				return '%' + m_OptionInt + "llu";
			}
			case KCMDT_FLOAT32:
			{
				return '%' + m_OptionFloat + "f";
			}
			case KCMDT_FLOAT64:
			{
				return '%' + m_OptionFloat + "lf";
			}
			case KCMDT_BOOL:
			{
				return '%' + m_OptionBool + "b";
			}
			case KCMDT_STRING:
			{
				return '%' + m_OptionString + "s";
			}
		};
		return wxEmptyString;
	};
	
	if (!m_Pattern.IsEmpty())
	{
		wxString out = wxString(m_Pattern);
		out.Replace("$", FormatOption(), true);
		return out;
	}
	return FormatOption();
}
int KCMOptionsFormatter::GetFloatPrecision() const
{
	int index = m_OptionFloat.Find('.');
	if (index != wxNOT_FOUND)
	{
		return std::wcstoull(m_OptionFloat.wc_str() + index + 1, NULL, 10);
	}
	return 0;
}

wxString KCMOptionsFormatter::operator()(const wxString& v, KCMDataType type) const
{
	if (KConfigManager::IsFloatType(type))
	{
		double dValue = 0;
		v.ToCDouble(&dValue);
		if (type == KCMDT_FLOAT64)
		{
			return operator()(dValue);
		}
		else
		{
			return operator()((float)dValue);
		}
	}
	else if (KConfigManager::IsBoolType(type))
	{
		return operator()(KAux::StringToBool(v));
	}
	else if (KConfigManager::IsStringType(type))
	{
		return operator()(v);
	}
	else if (type == KCMDT_INT8 || type == KCMDT_INT16 || type == KCMDT_INT32)
	{
		long value = 0;
		v.ToCLong(&value);
		return operator()((int32_t)value);
	}
	else if (type == KCMDT_UINT8 || type == KCMDT_UINT16 || type == KCMDT_UINT32)
	{
		unsigned long value = 0;
		v.ToCULong(&value);
		return operator()((uint32_t)value);
	}
	else if (type == KCMDT_INT64)
	{
		long long value = 0;
		v.ToLongLong(&value);
		return operator()((int64_t)value);
	}
	else if (type == KCMDT_UINT64)
	{
		unsigned long long value = 0;
		v.ToULongLong(&value);
		return operator()((uint64_t)value);
	}
	return v;
}
wxString KCMOptionsFormatter::operator()(const wxString& v) const
{
	return wxString::Format(GetFormatOption(KCMDT_STRING), v);
}
wxString KCMOptionsFormatter::operator()(const char* v) const
{
	return wxString::Format(GetFormatOption(KCMDT_STRING), v);
}
wxString KCMOptionsFormatter::operator()(const wchar_t* v) const
{
	return wxString::Format(GetFormatOption(KCMDT_STRING), v);
}
wxString KCMOptionsFormatter::operator()(int8_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_INT8), v);
}
wxString KCMOptionsFormatter::operator()(uint8_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_UINT8), v);
}
wxString KCMOptionsFormatter::operator()(int16_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_INT16), v);
}
wxString KCMOptionsFormatter::operator()(uint16_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_UINT16), v);
}
wxString KCMOptionsFormatter::operator()(int32_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_INT32), v);
}
wxString KCMOptionsFormatter::operator()(uint32_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_UINT32), v);
}
wxString KCMOptionsFormatter::operator()(int64_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_INT64), v);
}
wxString KCMOptionsFormatter::operator()(uint64_t v) const
{
	return wxString::Format(GetFormatOption(KCMDT_UINT64), v);
}
wxString KCMOptionsFormatter::operator()(float v) const
{
	return wxString::Format(GetFormatOption(KCMDT_FLOAT32), v);
}
wxString KCMOptionsFormatter::operator()(double v) const
{
	return wxString::Format(GetFormatOption(KCMDT_FLOAT64), v);
}
wxString KCMOptionsFormatter::operator()(bool v) const
{
	wxString sOption = GetFormatOption(KCMDT_BOOL);
	if (sOption.Length() >= 2)
	{
		if (sOption[0] == L'%')
		{
			auto c = sOption[1];
			if (c == L's')
			{
				return v ? "true" : "false";
			}
			else if (c == L'S')
			{
				return v ? "TRUE" : "FALSE";
			}
		}
	}
	return std::to_wstring((int)v);
}
