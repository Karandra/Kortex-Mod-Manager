#include "stdafx.h"
#include "KCMConfigEntry.h"
#include "KCMFileEntry.h"
#include "KConfigManager.h"
#include "Profile/KProfile.h"
#include "KApp.h"
#include <KxFramework/KxString.h>

KCMConfigEntryBase::KCMConfigEntryBase(KCMFileEntry* fileEntry)
	:m_FileEntry(fileEntry)
{
}
KCMConfigEntryBase::~KCMConfigEntryBase()
{
}

//////////////////////////////////////////////////////////////////////////
KCMConfigEntryPath::KCMConfigEntryPath(KCMFileEntry* fileEntry, const wxString& path)
	:KCMConfigEntryBase(fileEntry), m_Path(path)
{
}
KCMConfigEntryPath::~KCMConfigEntryPath()
{
}

void KCMConfigEntryPath::RemovePath(KxINI& tDocument)
{
	tDocument.RemoveSection(m_Path);
}
void KCMConfigEntryPath::RemovePath(KxXMLDocument& tDocument)
{
}

wxString KCMConfigEntryPath::GetFullPathFor(const wxString& name) const
{
	return KxString::Join({m_FileEntry->GetProfileEntry()->GetFileName(), m_Path, name}, m_FileEntry->GetConfigManager()->GetCategorySeparator());
}

//////////////////////////////////////////////////////////////////////////
KCMDataSubType KCMConfigEntryStd::GetSubType(KxXMLNode& node)
{
	wxString sSubType = node.GetAttribute("SubType");
	if (sSubType == "DoubleValue")
	{
		return KCMDST_DOUBLE_VALUE;
	}
	else if (sSubType == "VirtualKey")
	{
		return KCMDST_VIRTUAL_KEY;
	}
	else if (sSubType == "Array")
	{
		return KCMDST_ARRAY;
	}
	else if (sSubType == "FileBrowse")
	{
		return KCMDST_FILE_BROWSE;
	}
	return KCMDST_NONE;
}
wxString KCMConfigEntryStd::FormatToDisplay(const DataType& data, KCMDataType type) const
{
	if (data.second)
	{
		if (!data.first.IsEmpty())
		{
			if (KConfigManager::IsBoolType(type))
			{
				if (data.first[0] == L'0' || data.first.IsSameAs("false", false))
				{
					return "false";
				}
				else
				{
					return "true";
				}
			}
			else
			{
				return data.first;
			}
		}
		else
		{
			return KApp::Get().ExpandVariables("<$T(ConfigManager.View.EmptyStringValue)>");
		}
	}
	return KApp::Get().ExpandVariables("<$T(ID_NONE)>");
}

void KCMConfigEntryStd::LoadMain(KxXMLNode& node)
{
	m_Category = node.GetAttribute("Category");
	m_Path = node.GetAttribute("Path");
	m_Name = node.GetAttribute("Name");
	m_Type = KConfigManager::GetTypeID(node.GetAttribute("Type"));

	m_Label = node.GetAttribute("Label");
	if (!m_Label.IsEmpty())
	{
		m_Label = KApp::Get().ExpandVariables(m_Label);
	}
	else
	{
		bool bTranslated = false;
		m_Label = KAux::StrOr(KxTranslation::GetString(wxString::Format("GameConfig.Values.%s", m_Name), &bTranslated), m_Name);
		if (bTranslated)
		{
			m_Label = KApp::Get().ExpandVariables(m_Label);
		}
	}
}
void KCMConfigEntryStd::LoadSamples(KxXMLNode& node)
{
	if (!KConfigManager::IsBoolType(m_Type))
	{
		KxXMLNode tSamplesNode = node.GetFirstChildElement("SampleValues");
		if (tSamplesNode.IsOK())
		{
			bool bSortDefined = false;
			m_SortSampleValues = tSamplesNode.GetAttributeBool("Sort", &bSortDefined) || !bSortDefined;

			wxString sFillFunction = tSamplesNode.GetAttribute("FillFunction");
			if (!sFillFunction.IsEmpty())
			{
				auto pFunction = GetFileEntry()->GetConfigManager()->OnQueryFillFunction(sFillFunction);
				if (pFunction)
				{
					m_SampleValues = pFunction(this, node);
				}
			}
			CreateSamplesFromSequence(tSamplesNode);
			if (tSamplesNode.HasChildren())
			{
				LoadSamplesFromArray(tSamplesNode);
			}

			CleanSamplesArray();
			if (m_SortSampleValues)
			{
				std::sort(m_SampleValues.begin(), m_SampleValues.end(), &KCMSampleValue::SortComparator);
			}
			LoadEditableBehavior(tSamplesNode);
		}
	}
}
void KCMConfigEntryStd::LoadEditableBehavior(KxXMLNode& node)
{
	int nEditable = node.GetAttributeInt("Editable", -1);
	switch (nEditable)
	{
		case KCMEB_EDITABLE:
		case KCMEB_NON_EDITABLE:
		{
			m_EditableBehavior = (KCMEditableBehavior)nEditable;
			break;
		}
		default:
		{
			m_EditableBehavior = KCMEB_CONTEXT;
		}
	};
}
void KCMConfigEntryStd::LoadSamplesFromArray(KxXMLNode& node)
{
	for (KxXMLNode entryNode = node.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		m_SampleValues.push_back(OnLoadSampleValue(entryNode));
	}
}
KCMSampleValue KCMConfigEntryStd::OnLoadSampleValue(KxXMLNode& node) const
{
	return KCMSampleValue(this, node);
}
void KCMConfigEntryStd::CreateSamplesFromSequence(KxXMLNode& node)
{
	if (node.HasAttribute("Min"))
	{
		if (KConfigManager::IsIntType(m_Type))
		{
			int64_t nMin = node.GetAttributeInt("Min");
			auto max = node.GetAttributeInt("Max", nMin);
			auto nStep = node.GetAttributeInt("Step", 1);

			for (auto i = nMin; i <= max; i += nStep)
			{
				m_SampleValues.push_back(KCMSampleValue(this, GetFormatter()(i)));
			}

			m_MinValue.first = nMin;
			m_MinValue.second = true;
			m_MaxValue.first = max;
			m_MaxValue.second = true;
		}
		else if (KConfigManager::IsFloatType(m_Type))
		{
			auto nMin = node.GetAttributeFloat("Min");
			auto max = node.GetAttributeFloat("Max", nMin);
			auto nStep = node.GetAttributeFloat("Step", 0.1);

			auto value = nMin;
			auto nLast = nMin;
			while (value <= max)
			{
				m_SampleValues.push_back(KCMSampleValue(this, GetFormatter()(value)));
				nLast = value;
				value += nStep;
			}
			if (max - nLast > 0.001)
			{
				m_SampleValues.push_back(KCMSampleValue(this, GetFormatter()(max)));
			}

			m_MinValue.first = nMin;
			m_MinValue.second = true;
			m_MaxValue.first = max;
			m_MaxValue.second = true;
		}
	}
}
void KCMConfigEntryStd::CleanSamplesArray()
{
	std::unordered_set<wxString> tHash;
	if (!m_SampleValues.empty())
	{
		for (size_t i = m_SampleValues.size() - 1; i != 0; i--)
		{
			if (tHash.count(m_SampleValues[i].GetValue()))
			{
				m_SampleValues.erase(m_SampleValues.begin() + i);
			}
			else
			{
				tHash.emplace(m_SampleValues[i].GetValue());
			}
		}
	}
}

KCMConfigEntryStd::KCMConfigEntryStd(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions)
	:KCMConfigEntryPath(fileEntry, wxEmptyString), m_Formatter(tDefaultOptions)
{
}
void KCMConfigEntryStd::Create(KxXMLNode& node)
{
	m_Formatter = KCMOptionsFormatter::LoadFormatOptions(node, m_Formatter);

	LoadMain(node);
	LoadSamples(node);
}
void KCMConfigEntryStd::Create(const wxString& path, const wxString& name, KCMDataType type)
{
	m_Path = path;
	m_Category = GetFileEntry()->GetProfileEntry()->GetFileName() + KConfigManager::GetCategorySeparator() + m_Path;
	m_Name = name;
	m_Label = m_Name;
	m_Type = type;
}
KCMConfigEntryStd::~KCMConfigEntryStd()
{
}

void KCMConfigEntryStd::SaveEntry(KxINI& tDocument) const
{
	tDocument.SetValue(m_Path, m_Name, GetData(true));
}
void KCMConfigEntryStd::SaveEntry(KxXMLDocument& tDocument) const
{
}
void KCMConfigEntryStd::LoadEntry(const KxINI& tDocument)
{
	m_Data.second = tDocument.HasValue(m_Path, m_Name);
	if (m_Data.second)
	{
		m_Data.first = tDocument.GetValue(m_Path, m_Name);
	}
}
void KCMConfigEntryStd::LoadEntry(const KxXMLDocument& tDocument)
{
}
void KCMConfigEntryStd::RemoveEntry(KxINI& tDocument)
{
	tDocument.RemoveValue(m_Path, m_Name);
}
void KCMConfigEntryStd::RemoveEntry(KxXMLDocument& tDocument)
{
}

bool KCMConfigEntryStd::IsEditable() const
{
	if (m_EditableBehavior == KCMEB_CONTEXT)
	{
		return m_Type == KCMDT_UNKNOWN || KConfigManager::IsFloatType(m_Type) || ((KConfigManager::IsStringType(m_Type) || KConfigManager::IsIntType(m_Type)) && m_SampleValues.empty());
	}
	return m_EditableBehavior == KCMEB_EDITABLE;
}
wxString KCMConfigEntryStd::GetTypeName() const
{
	return KConfigManager::GetTypeName(m_Type);
}
wxString KCMConfigEntryStd::GetDisplayTypeName() const
{
	if (m_Type == KCMDT_UNKNOWN)
	{
		return wxString::Format("<%s>", KConfigManager::GetTypeName(m_Type));
	}
	return GetTypeName();
}
wxString KCMConfigEntryStd::GetFullPath() const
{
	return GetFullPathFor(m_Name);
}
wxString KCMConfigEntryStd::GetDisplayData() const
{
	int index = FindDataInSamples();
	if (index != wxNOT_FOUND)
	{
		const wxString& label = GetSampleValues()[index].GetLabel();
		if (!label.IsEmpty())
		{
			return wxString::Format("%s - %s", FormatToDisplay(m_Data, m_Type), label);
		}
		return GetFormatter()(GetSampleValues()[index].GetValue(), m_Type);
	}
	return FormatToDisplay(m_Data, m_Type);
}
wxString KCMConfigEntryStd::OnDisplaySampleValue(const KCMSampleValue& value) const
{
	if (value.GetLabel() != value.GetValue() && !value.GetLabel().IsEmpty())
	{
		return wxString::Format("%s - %s", value.GetLabel(), GetFormatter()(value.GetValue(), m_Type));
	}
	return KAux::StrOr(GetFormatter()(value.GetValue(), m_Type), value.GetLabel());
}
int KCMConfigEntryStd::FindDataInSamples(const wxString& sSearchFor) const
{
	if (!m_SampleValues.empty())
	{
		wxString sData = sSearchFor.IsEmpty() ? GetData(false) : sSearchFor;
		auto tElement = std::find_if(m_SampleValues.begin(), m_SampleValues.end(), [&sData](const KCMSampleValue& item)
		{
			return sData == item.GetValue();
		});
		if (tElement != m_SampleValues.end())
		{
			return std::distance(m_SampleValues.begin(), tElement);
		}
	}
	return wxNOT_FOUND;
}
wxString KCMConfigEntryStd::GetData(bool bFormat) const
{
	if (bFormat)
	{
		return GetFormatter()(m_Data.first, m_Type);
	}
	return m_Data.first;
}
void KCMConfigEntryStd::SetData()
{
	m_Data.first = wxEmptyString;
	m_Data.second = false;
}
void KCMConfigEntryStd::SetData(const wxString& sData, bool bFormat)
{
	if (bFormat)
	{
		m_Data.first = GetFormatter()(sData, m_Type);
	}
	else
	{
		m_Data.first = sData;
	}
	m_Data.second = true;
}

bool KCMConfigEntryStd::GetDataBool() const
{
	if (KConfigManager::IsBoolType(m_Type) && m_Data.second)
	{
		return KAux::StringToBool(m_Data.first);
	}
	return false;
}
void KCMConfigEntryStd::SetDataBool(bool value)
{
	m_Data.second = true;
	m_Data.first = m_Formatter(value);
}

//////////////////////////////////////////////////////////////////////////
KCMSampleValue KCMConfigEntryDV::OnLoadSampleValue(KxXMLNode& node) const
{
	wxString label;
	if (node.HasAttribute("Label"))
	{
		label = KApp::Get().ExpandVariables(node.GetAttribute("Label"));
	}
	wxString value = node.GetValue();

	return KCMSampleValue(this, value, label);
}

KCMConfigEntryDV::KCMConfigEntryDV(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions)
	:KCMConfigEntryStd(fileEntry, tDefaultOptions)
{
}
void KCMConfigEntryDV::Create(KxXMLNode& node)
{
	m_Separator = node.GetFirstChildElement("Separator").GetValue();
	m_ValueName1 = node.GetFirstChildElement("ValueName1").GetValue();
	m_ValueName2 = node.GetFirstChildElement("ValueName2").GetValue();

	KCMConfigEntryStd::Create(node);
}
KCMConfigEntryDV::~KCMConfigEntryDV()
{
}

void KCMConfigEntryDV::SaveEntry(KxINI& tDocument) const
{
	tDocument.SetValue(GetPath(), m_ValueName1, m_Data1.first);
	tDocument.SetValue(GetPath(), m_ValueName2, m_Data2.first);
}
void KCMConfigEntryDV::LoadEntry(const KxINI& tDocument)
{
	auto LoadField = [this, &tDocument](DataType& tFieled, const wxString& sValueName)
	{
		tFieled.second = tDocument.HasValue(m_Path, sValueName);
		if (tFieled.second)
		{
			tFieled.first = tDocument.GetValue(m_Path, sValueName);
		}
	};

	LoadField(m_Data1, m_ValueName1);
	LoadField(m_Data2, m_ValueName2);
}

wxString KCMConfigEntryDV::GetDisplayData() const
{
	auto ToDisplay = [this]()
	{
		return FormatToDisplay(m_Data1, m_Type) + m_Separator + FormatToDisplay(m_Data2, m_Type);
	};

	int index = FindDataInSamples();
	if (index != wxNOT_FOUND)
	{
		const wxString& label = GetSampleValues()[index].GetLabel();
		if (!label.IsEmpty())
		{
			return wxString::Format("%s - %s", ToDisplay(), label);
		}
		return GetSampleValues()[index].GetValue();
	}
	return ToDisplay();
}
wxString KCMConfigEntryDV::OnDisplaySampleValue(const KCMSampleValue& value) const
{
	if (!value.GetLabel().IsEmpty())
	{
		return wxString::Format("%s - %s", value.GetValue(), value.GetLabel());
	}
	return value.GetValue();
}

bool KCMConfigEntryDV::HasData() const
{
	return m_Data1.second && m_Data2.second;
}
wxString KCMConfigEntryDV::GetData(bool bFormat) const
{
	if (bFormat)
	{
		return GetFormatter()(m_Data1.first, m_Type) + m_Separator + GetFormatter()(m_Data2.first, m_Type);
	}
	else
	{
		return m_Data1.first + m_Separator + m_Data2.first;
	}
}
void KCMConfigEntryDV::SetData()
{
	m_Data1.first.Clear();
	m_Data1.second = false;

	m_Data2.first.Clear();
	m_Data2.second = false;
}
void KCMConfigEntryDV::SetData(const wxString& sData, bool bFormat)
{
	SetData();

	KxStringVector data = KxString::Split(sData, m_Separator);
	if (data.size() >= 1)
	{
		m_Data1.first = bFormat ? GetFormatter()(data[0], m_Type) : data[0];
		m_Data1.second = true;
	}
	if (data.size() >= 2)
	{
		m_Data2.first = bFormat ? GetFormatter()(data[1], m_Type) : data[1];
		m_Data2.second = true;
	}
}

//////////////////////////////////////////////////////////////////////////
KCMConfigEntryVK::KCMConfigEntryVK(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions)
	:KCMConfigEntryStd(fileEntry, tDefaultOptions), m_DataVK(std::make_pair(WXK_NONE, false))
{
}
KCMConfigEntryVK::~KCMConfigEntryVK()
{
}

void KCMConfigEntryVK::LoadEntry(const KxINI& tDocument)
{
	SetData(tDocument.GetValue(m_Path, m_Name), false);
}

wxString KCMConfigEntryVK::GetDisplayData() const
{
	if (HasData())
	{
		auto info = GetFileEntry()->GetConfigManager()->GetVirtualKeyInfo(GetDataKeyCode());
		return wxString::Format("%s - %d", info.second.IsEmpty() ? info.first : info.second, (int)GetDataKeyCode());
	}
	return KApp::Get().ExpandVariables("<$T(ID_NONE)>");
}
wxString KCMConfigEntryVK::OnDisplaySampleValue(const KCMSampleValue& value) const
{
	if (!value.GetLabel().IsEmpty())
	{
		return wxString::Format("%s - %s", value.GetValue(), value.GetLabel());
	}
	return value.GetValue();
}

wxString KCMConfigEntryVK::GetData(bool bFormat) const
{
	return wxString::Format("%d", (int)GetDataKeyCode());
}
void KCMConfigEntryVK::SetData(const wxString& sData, bool bFormat)
{
	unsigned long value = WXK_NONE;
	m_DataVK.second = sData.ToCULong(&value);
	if (m_DataVK.second)
	{
		m_DataVK.first = (wxKeyCode)value;
	}
}

//////////////////////////////////////////////////////////////////////////
void KCMConfigEntryArray::SaveEntry(KxINI& tDocument) const
{
	tDocument.SetValue(m_Path, m_Name, GetData(false));
}
void KCMConfigEntryArray::LoadEntry(const KxINI& tDocument)
{
	SetData(tDocument.GetValue(m_Path, m_Name), false);
}

KCMConfigEntryArray::KCMConfigEntryArray(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions)
	:KCMConfigEntryStd(fileEntry, tDefaultOptions)
{
}
void KCMConfigEntryArray::Create(KxXMLNode& node)
{
	m_Separator = node.GetFirstChildElement("Separator").GetValue();

	KCMConfigEntryStd::Create(node);
}
KCMConfigEntryArray::~KCMConfigEntryArray()
{
}

wxString KCMConfigEntryArray::GetDisplayData() const
{
	if (HasData())
	{
		KxStringVector tLabels;
		for (const auto& v: m_Values)
		{
			int index = FindDataInSamples(v);
			if (index != wxNOT_FOUND)
			{
				const KCMSampleValue& value = GetSampleValues()[index];
				if (!value.GetLabel().IsEmpty())
				{
					tLabels.emplace_back(value.GetLabel());
				}
				else
				{
					tLabels.emplace_back(value.GetValue());
				}
			}
		}
		return KxString::Join(tLabels, ", ");
	}
	return FormatToDisplay(std::make_pair(wxEmptyString, false));
}
wxString KCMConfigEntryArray::OnDisplaySampleValue(const KCMSampleValue& value) const
{
	if (!value.GetLabel().IsEmpty())
	{
		return wxString::Format("%s - %s", value.GetValue(), value.GetLabel());
	}
	return value.GetValue();
}
wxString KCMConfigEntryArray::GetData(bool bFormat) const
{
	if (bFormat)
	{
		KxStringVector array;
		for (const wxString& v: m_Values)
		{
			GetFormatter()(array.emplace_back(v));
		}
		return KxString::Join(array, m_Separator);
	}
	else
	{
		return KxString::Join(m_Values, m_Separator);
	}
}
void KCMConfigEntryArray::SetData()
{
	m_Values.clear();
}
void KCMConfigEntryArray::SetData(const wxString& sData, bool bFormat)
{
	SetData();
	m_Values = KxString::Split(sData, m_Separator, false);

	if (bFormat)
	{
		for (wxString& v: m_Values)
		{
			v = GetFormatter()(v);
		}
	}

	for (wxString& v: m_Values)
	{
		if (FindDataInSamples(v) == wxNOT_FOUND)
		{
			GetSampleValuesEditable().emplace_back(KCMSampleValue(this, v));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
KCMConfigEntryFileBrowse::KCMConfigEntryFileBrowse(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions)
	:KCMConfigEntryStd(fileEntry, tDefaultOptions)
{
}
void KCMConfigEntryFileBrowse::Create(KxXMLNode& node)
{
	KCMConfigEntryStd::Create(node);

	m_IsFolder = node.GetAttributeBool("IsFolder");
}
KCMConfigEntryFileBrowse::~KCMConfigEntryFileBrowse()
{
}
