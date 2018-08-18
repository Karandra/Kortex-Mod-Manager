#pragma once
#include "stdafx.h"
enum KCMDataType;

enum KCMOFMode
{
	KCMOF_MODE_DEFAULT,
	KCMOF_MODE_SAVE,
	KCMOF_MODE_LOAD,
};

class KCMOptionsFormatter
{
	public:
		static KCMOptionsFormatter LoadFormatOptions(KxXMLNode& node, const KCMOptionsFormatter& tDefaultOptions);

	private:
		wxString m_Pattern;
		wxString m_OptionInt;
		wxString m_OptionFloat;
		wxString m_OptionString;
		wxString m_OptionBool;

	public:
		KCMOptionsFormatter();
		KCMOptionsFormatter(KxXMLNode& node);
		void Load(KxXMLNode& node);

	public:
		wxString GetFormatOption(KCMDataType type) const;
		int GetFloatPrecision() const;

		wxString operator()(const wxString& v, KCMDataType type) const;
		wxString operator()(const wxString& v) const;
		wxString operator()(const char* v) const;
		wxString operator()(const wchar_t* v) const;
		wxString operator()(int8_t v) const;
		wxString operator()(uint8_t v) const;
		wxString operator()(int16_t v) const;
		wxString operator()(uint16_t v) const;
		wxString operator()(int32_t v) const;
		wxString operator()(uint32_t v) const;
		wxString operator()(int64_t v) const;
		wxString operator()(uint64_t v) const;
		wxString operator()(float v) const;
		wxString operator()(double v) const;
		wxString operator()(bool v) const;
};
