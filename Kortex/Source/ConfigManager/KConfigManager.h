#pragma once
#include "stdafx.h"
#include "KManager.h"
#include "KCMConfigEntry.h"
#include "KCMFileEntry.h"
#include "KCMSampleValue.h"
#include "KCMOptionsFormatter.h"
#include "KCMIDataProvider.h"
#include "KCMValueTypeDetector.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxSingleton.h>
class KConfigManagerConfig;
class KWorkspace;
class KxXMLNode;
enum KPGCFileID;

class KConfigManager: public KManager//, public KxSingletonPtr<KConfigManager>
{
	private:
		typedef std::unordered_map<wxString, KCMDataType> NameToTypeMap;
		typedef std::unordered_map<wxString, wxString> CategoriesMap;
		typedef std::pair<wxString, wxString> VirtualKeyMapInfo;
		typedef std::unordered_map<wxKeyCode, VirtualKeyMapInfo> VirtualKeyMap;
		static const NameToTypeMap ms_NameToTypeMap;

	public:
		static const wxString& GetCategorySeparator();
		static wxString GetConfigFile(const wxString& fileName);
		static wxString GetConfigFileForTemplate(const wxString& templateID);
		static const KConfigManagerConfig* GetGameConfig();

		static bool IsIntType(KCMDataType type);
		static bool IsSignedIntType(KCMDataType type);
		static bool IsUnsignedIntType(KCMDataType type);
		static bool IsFloatType(KCMDataType type);
		static bool IsNumericType(KCMDataType type);
		static bool IsBoolType(KCMDataType type);
		static bool IsStringType(KCMDataType type);

		static std::pair<int64_t, int64_t> GetMinMaxSignedValue(KCMDataType type);
		static std::pair<int64_t, int64_t> GetMinMaxUnsignedValue(KCMDataType type);
		static std::pair<double, double> GetMinMaxFloatValue(KCMDataType type);

		static KCMDataType GetTypeID(const wxString& name);
		static wxString GetTypeName(KCMDataType type);

	public:
		static KCMTypeDetector GetTypeDetectorID(const wxString& name);

		typedef KCMSampleValueArray(*FillFunnctionType)(KCMConfigEntryStd*, KxXMLNode&);
		static KCMSampleValueArray FF_GetVideoAdapterList(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_GetVideoModesList(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_GetVirtualKeys(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_FindFiles(KCMConfigEntryStd* configEntry, KxXMLNode& node);

	protected:
		void LoadMainFile(KxXMLDocument& xml);
		void LoadAdditionalFile(KxXMLDocument& xml, bool bAllowENB = false);

		void LoadFormatOptions(KxXMLDocument& xml);
		void InitTypeDetectors(KxXMLDocument& xml);
		void LoadCategories(KxXMLDocument& xml);
		void LoadFileRecords(KxXMLDocument& xml, bool bAllowENB = false);

		void LoadVirtualKeys();

	protected:
		wxString m_FilePath;
		KxXMLDocument m_XML;
		std::vector<std::pair<wxString, KxXMLDocument*>> m_AdditionalXML;

	private:
		KWorkspace* m_Workspace = NULL;
		KCMOptionsFormatter m_Formatter;
		KCMHungarianNotationDetector m_Detector_HungarianNotation;
		KCMDataAnalysisDetector m_Detector_DataAnalysis;
		CategoriesMap m_Categories;
		VirtualKeyMap m_VirtualKeys;
		KCMFileEntryArray m_Files;
		std::unordered_map<KPGCFileID, KCMIDataProvider*> m_Providers;

	protected:
		KConfigManager(KWorkspace* workspace = NULL);

	public:
		KConfigManager(KWorkspace* workspace, const wxString& templateID);
		virtual ~KConfigManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_GEAR;
		}
		virtual KWorkspace* GetWorkspace() const override
		{
			return m_Workspace;
		}
		virtual void SetWorkspace(KWorkspace* workspace)
		{
			if (m_Workspace == NULL)
			{
				m_Workspace = workspace;
			}
		}

	public:
		void AddFile(const wxString& fileName, bool bAllowENB = false);
		void AddENB();

		size_t GetEntriesCount() const
		{
			return m_Files.size();
		}
		KCMFileEntry* GetEntryAt(size_t i) const
		{
			if (i < m_Files.size())
			{
				return m_Files[i];
			}
			return NULL;
		}
		KCMFileEntry* GetEntry(KPGCFileID id) const;
		const CategoriesMap& GetCategories() const
		{
			return m_Categories;
		}
		const KCMOptionsFormatter& GetFormatter() const
		{
			return m_Formatter;
		}
		const VirtualKeyMap& GetVirtualKeys() const
		{
			return m_VirtualKeys;
		}
		const VirtualKeyMapInfo& GetVirtualKeyInfo(wxKeyCode keyCode) const;

		virtual void Clear();
		virtual void Reload();

		virtual FillFunnctionType OnQueryFillFunction(const wxString& name);
		virtual KCMIDataProvider* OnQueryDataProvider(const KCMFileEntry* fileEntry);
		virtual KCMValueTypeDetector* OnQueryTypeDetector(KCMTypeDetector id);
};
