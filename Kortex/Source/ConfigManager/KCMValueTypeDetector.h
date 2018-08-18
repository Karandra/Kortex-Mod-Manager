#pragma once
#include "stdafx.h"

enum KCMTypeDetector
{
	KCM_DETECTOR_INVALID = -1,

	KCM_DETECTOR_HUNGARIAN_NOTATION,
	KCM_DETECTOR_DATA_ANALYSIS,
};

enum KCMDataType;
class KCMValueTypeDetector
{
	public:
		KCMValueTypeDetector() {}
		virtual ~KCMValueTypeDetector() {}

	public:
		virtual KCMDataType operator()(const wxString& sValueName, const wxString& sValueData) = 0;
		virtual void Clear() = 0;
};

class KCMHungarianNotationDetector: public KCMValueTypeDetector
{
	private:
		std::unordered_map<wxString, KCMDataType> m_TypesMap;
		
	public:
		void Init(KxXMLNode& node);
		virtual KCMDataType operator()(const wxString& sValueName, const wxString& sValueData) override;
		virtual void Clear() override
		{
			m_TypesMap.clear();
		}
};

class KCMDataAnalysisDetector: public KCMValueTypeDetector
{
	public:
		virtual KCMDataType operator()(const wxString& sValueName, const wxString& sValueData) override;
		virtual void Clear() override
		{
		}
};