#include "stdafx.h"
#include "NativeSerializer.h"
#include "Serializer.h"
#include "ModPackageProject.h"
#include "ConfigSection.h"
#include "InfoSection.h"
#include "InterfaceSection.h"
#include "FileDataSection.h"
#include "RequirementsSection.h"
#include "ComponentsSection.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/PackageManager.hpp>
#include "GameInstance/IGameInstance.h"
#include "Utility/KAux.h"

namespace Kortex::PackageProject
{
	namespace
	{
		void WriteCondition(const Condition& condition, KxXMLNode& conditionNode, bool writeOperator)
		{
			if (writeOperator)
			{
				conditionNode.SetAttribute("Operator", ModPackageProject::OperatorToString(condition.GetOperator()));
			}
	
			for (const FlagItem& flag: condition.GetFlags())
			{
				KxXMLNode flagNode = conditionNode.NewElement("Flag");
				flagNode.SetValue(flag.GetValue());
				flagNode.SetAttribute("Name", flag.GetName());
			}
		}
		void WriteConditionGroup(const ConditionGroup& conditionGroup, KxXMLNode& groupNode)
		{
			groupNode.SetAttribute("Operator", ModPackageProject::OperatorToString(conditionGroup.GetOperator()));
			for (const Condition& condition: conditionGroup.GetConditions())
			{
				if (condition.HasFlags())
				{
					WriteCondition(condition, groupNode.NewElement("Condition"), true);
				}
			}
		}
		
		void ReadCondition(Condition& condition, const KxXMLNode& conditionNode)
		{
			condition.SetOperator(ModPackageProject::StringToOperator(conditionNode.GetAttribute("Operator"), false, Operator::And));
			for (KxXMLNode node = conditionNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				condition.GetFlags().emplace_back(node.GetValue(), node.GetAttribute("Name"));
			}
		}
		void ReadConditionGroup(ConditionGroup& conditionGroup, const KxXMLNode& groupNode)
		{
			conditionGroup.SetOperator(ModPackageProject::StringToOperator(groupNode.GetAttribute("Operator"), false, Operator::And));
			for (KxXMLNode conditionNode = groupNode.GetFirstChildElement(); conditionNode.IsOK(); conditionNode = conditionNode.GetNextSiblingElement())
			{
				Condition& condition = conditionGroup.GetConditions().emplace_back();
				ReadCondition(condition, conditionNode);
				if (!condition.HasFlags())
				{
					conditionGroup.GetConditions().pop_back();
				}
			}
		}
	
		template<class T> void WriteLabeledValueArray(const KLabeledValue::Vector& array, KxXMLNode& arrayNode, const T& Func, bool isCDATA = false)
		{
			for (const KLabeledValue& value: array)
			{
				KxXMLNode elementNode = arrayNode.NewElement("Entry");
	
				elementNode.SetValue(Func(value));
				if (value.HasLabel())
				{
					elementNode.SetAttribute("Name", value.GetLabel());
				}
			}
		}
	}
}

namespace Kortex::PackageProject
{
	void NativeSerializer::ReadBase()
	{
		KxXMLNode baseNode = m_XML.QueryElement("Package");
		if (baseNode.IsOK())
		{
			m_ProjectLoad->SetFormatVersion(baseNode.GetAttribute("FormatVersion"));
			m_ProjectLoad->SetModID(baseNode.GetAttribute("ID"));
			
			KxXMLNode targetProfileNode = baseNode.GetFirstChildElement("TargetProfile");
			m_ProjectLoad->SetTargetProfileID(targetProfileNode.GetAttribute("ID"));
		}
	}
	void NativeSerializer::ReadConfig()
	{
		KxXMLNode configNode = m_XML.QueryElement("Package/PackageConfig");
		if (configNode.IsOK())
		{
			ConfigSection& config = m_ProjectLoad->GetConfig();
	
			config.SetInstallPackageFile(configNode.GetFirstChildElement("InstallPackageFile").GetValue());
			config.SetCompressionMethod(configNode.GetFirstChildElement("CompressionMethod").GetValue());
			config.SetCompressionLevel(configNode.GetFirstChildElement("CompressionLevel").GetValueInt());
			config.SetCompressionDictionarySize(configNode.GetFirstChildElement("CompressionDictionarySize").GetValueInt());
			config.SetUseMultithreading(configNode.GetFirstChildElement("CompressionUseMultithreading").GetValueBool());
			config.SetSolidArchive(configNode.GetFirstChildElement("CompressionSolidArchive").GetValueBool());
		}
	}
	void NativeSerializer::ReadInfo()
	{
		KxXMLNode infoNode = m_XML.QueryElement("Package/Info");
		if (infoNode.IsOK())
		{
			InfoSection& info = m_ProjectLoad->GetInfo();
	
			// Basic info
			info.SetName(infoNode.GetFirstChildElement("Name").GetValue());
			info.SetTranslatedName(infoNode.GetFirstChildElement("TranslatedName").GetValue());
			info.SetVersion(infoNode.GetFirstChildElement("Version").GetValue());
			info.SetAuthor(infoNode.GetFirstChildElement("Author").GetValue());
			info.SetTranslator(infoNode.GetFirstChildElement("Translator").GetValue());
			info.SetDescription(infoNode.GetFirstChildElement("Description").GetValue());
	
			// Custom info
			KAux::LoadLabeledValueArray(info.GetCustomFields(), infoNode.GetFirstChildElement("Custom"));
	
			// Source
			using namespace Kortex::NetworkManager;
			Kortex::ModSourceStore& store = info.GetModSourceStore();
			store.LoadAssign(infoNode.GetFirstChildElement("Source"));
	
			// Documents
			KAux::LoadLabeledValueArray(info.GetDocuments(), infoNode.GetFirstChildElement("Documents"), "Name");
	
			// Tags
			Kortex::ModTagStore& tagStore = info.GetTagStore();
			for (KxXMLNode node = infoNode.GetFirstChildElement("Tags"); node.IsOK(); node = node.GetNextSiblingElement())
			{
				tagStore.AddTag(Kortex::ModTagManager::DefaultTag(node.GetValue()));
			}
		}
	}
	void NativeSerializer::ReadInterface()
	{
		KxXMLNode interfaceNode = m_XML.QueryElement("Package/Interface");
		if (interfaceNode.IsOK())
		{
			InterfaceSection& interfaceConfig = m_ProjectLoad->GetInterface();
			TitleConfig& titleConfig = interfaceConfig.GetTitleConfig();
	
			// Read customization
			KxXMLNode titleConfigNode = interfaceNode.GetFirstChildElement("Caption");
			if (titleConfigNode.IsOK())
			{
				titleConfig.SetAlignment((wxAlignment)titleConfigNode.GetAttributeInt("Alignment", TitleConfig::ms_InvalidAlignment));
				
				int64_t colorValue = titleConfigNode.GetAttributeInt("Color", -1);
				if (colorValue != -1)
				{
					titleConfig.SetColor(KxColor::FromRGBA(colorValue));
				}
			}
	
			// Read special images config
			auto ReadImageConfig = [](const KxXMLNode& node) -> ImageItem
			{
				ImageItem entry;
				if (node.IsOK())
				{
					entry.SetPath(node.GetAttribute("Path"));
					entry.SetVisible(node.GetAttributeBool("Visible", true));
					entry.SetSize(wxSize(node.GetAttributeInt("Width", wxDefaultCoord), node.GetAttributeInt("Height", wxDefaultCoord)));
					entry.SetDescription(node.GetFirstChildElement("Description").GetValue());
				}
				return entry;
			};
			interfaceConfig.SetMainImage(ReadImageConfig(interfaceNode.GetFirstChildElement("MainImage")).GetPath());
			interfaceConfig.SetHeaderImage(ReadImageConfig(interfaceNode.GetFirstChildElement("HeaderImage")).GetPath());
	
			// Read images list
			KxXMLNode imagesNode = interfaceNode.GetFirstChildElement("Images");
			for (KxXMLNode node = imagesNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				interfaceConfig.GetImages().emplace_back(ReadImageConfig(node));
			}
		}
	}
	void NativeSerializer::ReadFiles()
	{
		KxXMLNode fileDataNode = m_XML.QueryElement("Package/Files");
		if (fileDataNode.IsOK())
		{
			FileDataSection& fileData = m_ProjectLoad->GetFileData();
	
			// Folder
			for (KxXMLNode folderNode = fileDataNode.GetFirstChildElement(); folderNode.IsOK(); folderNode = folderNode.GetNextSiblingElement())
			{
				FileItem* fileEntry = nullptr;
				FolderItem* folderEntry = nullptr;
				if (folderNode.GetName() == "Folder")
				{
					folderEntry = new FolderItem();
					fileEntry = fileData.AddFolder(folderEntry);
				}
				else
				{
					fileEntry = fileData.AddFile(new FileItem());
				}
	
				fileEntry->SetID(folderNode.GetAttribute("ID"));
				fileEntry->SetSource(folderNode.GetAttribute("Source"));
				fileEntry->SetDestination(folderNode.GetAttribute("Destination"));
				fileEntry->SetPriority(folderNode.GetAttributeInt("Priority", FileDataSection::ms_DefaultPriority));
	
				if (m_AsProject && folderEntry)
				{
					for (KxXMLNode fileNode = folderNode.GetFirstChildElement(); fileNode.IsOK(); fileNode = fileNode.GetNextSiblingElement())
					{
						FolderItemElement& fileEntry = folderEntry->GetFiles().emplace_back(FolderItemElement());
						fileEntry.SetDestination(fileNode.GetValue());
						fileEntry.SetSource(fileNode.GetAttribute("Source"));
					}
				}
			}
		}
	}
	void NativeSerializer::ReadRequirements()
	{
		RequirementsSection& requirements = m_ProjectLoad->GetRequirements();
	
		KxXMLNode requirementsNode = m_XML.QueryElement("Package/Requirements");
		if (requirementsNode.IsOK())
		{
			KAux::LoadStringArray(requirements.GetDefaultGroup(), requirementsNode.GetFirstChildElement("DefaultGroups"));
	
			for (KxXMLNode groupNode = requirementsNode.GetFirstChildElement("Groups").GetFirstChildElement(); groupNode.IsOK(); groupNode = groupNode.GetNextSiblingElement())
			{
				RequirementGroup* requirementGroup = requirements.GetGroups().emplace_back(std::make_unique<RequirementGroup>()).get();
				requirementGroup->SetID(groupNode.GetAttribute("ID"));
				requirementGroup->SetOperator(ModPackageProject::StringToOperator(groupNode.GetAttribute("Operator"), false, requirements.ms_DefaultGroupOperator));
	
				for (KxXMLNode entryNode = groupNode.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
				{
					ReqType type = requirements.StringToTypeDescriptor(entryNode.GetAttribute("Type"));
	
					RequirementItem* entry = requirementGroup->GetEntries().emplace_back(std::make_unique<RequirementItem>(type)).get();
					entry->SetID(entryNode.GetAttribute("ID"));
					entry->SetName(entryNode.GetFirstChildElement("Name").GetValue());
	
					// Object
					KxXMLNode objectNode = entryNode.GetFirstChildElement("Object");
					entry->SetObject(objectNode.GetValue());
					entry->SetObjectFunction(requirements.StringToObjectFunction(objectNode.GetAttribute("Function")));
	
					// Version
					KxXMLNode versionNode = entryNode.GetFirstChildElement("Version");
					entry->SetRequiredVersion(versionNode.GetValue());
					entry->SetRVFunction(ModPackageProject::StringToOperator(versionNode.GetAttribute("Function"), false, requirements.ms_DefaultVersionOperator));
	
					// Description
					entry->SetDescription(entryNode.GetFirstChildElement("Description").GetValue());
	
					// Conform
					entry->ConformToTypeDescriptor();
				}
			}
		}
	}
	void NativeSerializer::ReadComponents()
	{
		ComponentsSection& components = m_ProjectLoad->GetComponents();
	
		KxXMLNode componentsNode = m_XML.QueryElement("Package/Components");
		if (componentsNode.IsOK())
		{
			// Read required files
			KAux::LoadStringArray(components.GetRequiredFileData(), componentsNode.GetFirstChildElement("RequiredFiles"));
	
			// Read steps
			for (KxXMLNode stepNode = componentsNode.GetFirstChildElement("Steps").GetFirstChildElement(); stepNode.IsOK(); stepNode = stepNode.GetNextSiblingElement())
			{
				auto& step = components.GetSteps().emplace_back(std::make_unique<ComponentStep>());
				step->SetName(stepNode.GetAttribute("Name"));
				ReadConditionGroup(step->GetConditionGroup(), stepNode.GetFirstChildElement("Conditions"));
	
				for (KxXMLNode groupNode = stepNode.GetFirstChildElement("Groups").GetFirstChildElement(); groupNode.IsOK(); groupNode = groupNode.GetNextSiblingElement())
				{
					auto& pSet = step->GetGroups().emplace_back(std::make_unique<ComponentGroup>());
					pSet->SetName(groupNode.GetAttribute("Name"));
					pSet->SetSelectionMode(components.StringToSelectionMode(groupNode.GetAttribute("SelectionMode")));
	
					for (KxXMLNode entryNode = groupNode.GetFirstChildElement("Entries").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
					{
						auto& entry = pSet->GetEntries().emplace_back(std::make_unique<ComponentItem>());
						entry->SetName(entryNode.GetFirstChildElement("Name").GetValue());
						entry->SetImage(entryNode.GetFirstChildElement("Image").GetAttribute("Path"));
						entry->SetDescription(entryNode.GetFirstChildElement("Description").GetValue());
	
						// Type descriptor
						KxXMLNode typeDescriptorNode = entryNode.GetFirstChildElement("TypeDescriptor");
						entry->SetTDDefaultValue(components.StringToTypeDescriptor(typeDescriptorNode.GetAttribute("DefaultValue")));
						entry->SetTDConditionalValue(components.StringToTypeDescriptor(typeDescriptorNode.GetAttribute("ConditionalValue"), TypeDescriptor::Invalid));
						
						if (m_ProjectLoad->GetFormatVersion() < KxVersion("1.3"))
						{
							KxXMLNode conditionsNode = typeDescriptorNode.GetFirstChildElement("Conditions");
							if (conditionsNode.IsOK())
							{
								ConditionGroup& conditionGroup = entry->GetTDConditionGroup();
								Condition& condition = conditionGroup.GetOrCreateFirstCondition();
								ReadCondition(condition, conditionsNode);
	
								conditionGroup.SetOperator(Operator::And);
								condition.SetOperator(Operator::And);
							}
						}
						else
						{
							ReadConditionGroup(entry->GetTDConditionGroup(), typeDescriptorNode.GetFirstChildElement("Conditions"));
						}
	
						// If condition list is empty and type descriptor values are equal, clear 'ConditionalValue'
						if (!entry->GetTDConditionGroup().HasConditions() && entry->GetTDDefaultValue() == entry->GetTDConditionalValue())
						{
							entry->SetTDConditionalValue(TypeDescriptor::Invalid);
						}
	
						KAux::LoadStringArray(entry->GetFileData(), entryNode.GetFirstChildElement("Files"));
						KAux::LoadStringArray(entry->GetRequirements(), entryNode.GetFirstChildElement("Requirements"));
						
						// Conditional flags
						KxXMLNode conditionalFlagsNode = entryNode.GetFirstChildElement("ConditionalFlags");
						if (!conditionalFlagsNode.IsOK())
						{
							// Old option name
							conditionalFlagsNode = entryNode.GetFirstChildElement("AssignedFlags");
						}
						ReadCondition(entry->GetConditionalFlags(), conditionalFlagsNode);
					}
				}
			}
	
			auto ReadConditionalSteps = [&componentsNode, &components](const wxString& sRootNodeName, const wxString& sNodeName)
			{
				for (KxXMLNode stepNode = componentsNode.GetFirstChildElement(sRootNodeName).GetFirstChildElement(); stepNode.IsOK(); stepNode = stepNode.GetNextSiblingElement())
				{
					auto& step = components.GetConditionalSteps().emplace_back(std::make_unique<ConditionalComponentStep>());
					ReadConditionGroup(step->GetConditionGroup(), stepNode.GetFirstChildElement("Conditions"));
					KAux::LoadStringArray(step->GetEntries(), stepNode.GetFirstChildElement(sNodeName));
				}
			};
			ReadConditionalSteps("ConditionalSteps", "Files");
		}
	}
	
	KxXMLNode NativeSerializer::WriteBase()
	{
		KxXMLNode baseNode = m_XML.NewElement("Package");
		baseNode.SetAttribute("FormatVersion", Kortex::ModPackagesModule::GetInstance()->GetModuleInfo().GetVersion());
		baseNode.SetAttribute("ID", m_ProjectSave->GetModID());
	
		KxXMLNode targetProfileNode = baseNode.NewElement("TargetProfile");
		targetProfileNode.SetAttribute("ID", Kortex::IGameInstance::GetActive()->GetGameID());
	
		return baseNode;
	}
	void NativeSerializer::WriteConfig(KxXMLNode& baseNode)
	{
		if (m_AsProject)
		{
			KxXMLNode configNode = baseNode.NewElement("PackageConfig");
			const ConfigSection& config = m_ProjectSave->GetConfig();
	
			configNode.NewElement("InstallPackageFile").SetValue(config.GetInstallPackageFile());
			configNode.NewElement("CompressionMethod").SetValue(config.GetCompressionMethod());
			configNode.NewElement("CompressionLevel").SetValue(config.GetCompressionLevel());
			configNode.NewElement("CompressionDictionarySize").SetValue(config.GetCompressionDictionarySize());
			configNode.NewElement("CompressionUseMultithreading").SetValue(config.IsMultithreadingUsed());
			configNode.NewElement("CompressionSolidArchive").SetValue(config.IsSolidArchive());
		}
	}
	void NativeSerializer::WriteInfo(KxXMLNode& baseNode)
	{
		KxXMLNode infoNode = baseNode.NewElement("Info");
		const InfoSection& info = m_ProjectSave->GetInfo();
	
		// Basic info
		infoNode.NewElement("Name").SetValue(info.GetName());
		infoNode.NewElement("Version").SetValue(info.GetVersion());
		infoNode.NewElement("Author").SetValue(info.GetAuthor());
	
		if (!info.GetTranslator().IsEmpty())
		{
			infoNode.NewElement("Translator").SetValue(info.GetTranslator());
		}
	
		if (!info.GetTranslatedName().IsEmpty())
		{
			infoNode.NewElement("TranslatedName").SetValue(info.GetTranslatedName());
		}
	
		infoNode.NewElement("Description").SetValue(info.GetDescription());
	
		// Custom info
		if (!info.GetCustomFields().empty())
		{
			KAux::SaveLabeledValueArray(info.GetCustomFields(), infoNode.NewElement("Custom"));
		}
	
		// Source
		KxXMLNode providerNode = infoNode.NewElement("Source");
		info.GetModSourceStore().Save(providerNode);
	
		// Documents
		if (!info.GetDocuments().empty())
		{
			WriteLabeledValueArray(info.GetDocuments(), infoNode.NewElement("Documents"), [this](const KLabeledValue& value)
			{
				return m_AsProject ? value.GetValue() : PathNameToPackage(value.GetValue(), ContentType::Documents);
			});
		}
	
		// Tags
		const Kortex::ModTagStore& tagStore = info.GetTagStore();
		if (!tagStore.IsEmpty())
		{
			KxXMLNode tagsNode = infoNode.NewElement("Tags");
			tagStore.Visit([&tagsNode](const Kortex::IModTag& tag)
			{
				tagsNode.NewElement("Entry").SetValue(tag.GetID());
				return true;
			});
		}
	}
	void NativeSerializer::WriteInterface(KxXMLNode& baseNode)
	{
		const InterfaceSection& interfaceConfig = m_ProjectSave->GetInterface();
		const TitleConfig& titleConfig = interfaceConfig.GetTitleConfig();
		KxXMLNode interfaceNode = baseNode.NewElement("Interface");
	
		// Write customization
		if (titleConfig.IsOK())
		{
			KxXMLNode node = interfaceNode.NewElement("Caption");
			if (titleConfig.HasAlignment())
			{
				node.SetAttribute("Alignment", (int64_t)titleConfig.GetAlignment());
			}
			if (titleConfig.HasColor())
			{
				node.SetAttribute("Color", (int64_t)titleConfig.GetColor().GetRGBA());
			}
		}
	
		// Write special images config
		auto WriteImageConfig = [this](KxXMLNode& rootNode, const wxString& name, const ImageItem* entry, bool isListEntry)
		{
			if (entry)
			{
				if (!isListEntry || entry->HasPath())
				{
					KxXMLNode node = rootNode.NewElement(name);
					node.SetAttribute("Path", m_AsProject ? entry->GetPath() : PathNameToPackage(entry->GetPath(), ContentType::Images));
					node.SetAttribute("Visible", entry->IsVisible());
	
					if (isListEntry)
					{
						if (entry->HasDescription())
						{
							node.NewElement("Description").SetValue(entry->GetDescriptionRaw());
						}
					}
					else
					{
						//node.SetAttribute("Width", entry->GetSize().GetWidth());
						//node.SetAttribute("Height", entry->GetSize().GetHeight());
					}
				}
			}
		};
		WriteImageConfig(interfaceNode, "MainImage", interfaceConfig.GetMainImageEntry(), false);
		WriteImageConfig(interfaceNode, "HeaderImage", interfaceConfig.GetHeaderImageEntry(), false);
	
		// Write images list
		if (!interfaceConfig.GetImages().empty())
		{
			KxXMLNode imagesNode = interfaceNode.NewElement("Images");
			for (const ImageItem& entry: interfaceConfig.GetImages())
			{
				WriteImageConfig(imagesNode, "Entry", &entry, true);
			}
		}
	}
	void NativeSerializer::WriteFiles(KxXMLNode& baseNode)
	{
		KxXMLNode fileDataNode = baseNode.NewElement("Files");
		const FileDataSection& fileData = m_ProjectSave->GetFileData();
	
		// Folders
		if (!fileData.GetData().empty())
		{
			for (const auto& entry: fileData.GetData())
			{
				const FolderItem* folderEntry = entry->ToFolderItem();
				KxXMLNode entryNode = fileDataNode.NewElement(folderEntry ? "Folder" : "File");
	
				if (m_AsProject)
				{
					if (entry->GetID() != entry->GetSource())
					{
						entryNode.SetAttribute("ID", entry->GetID());
					}
					entryNode.SetAttribute("Source", PathNameToPackage(entry->GetSource(), ContentType::FileData));
				}
				else
				{
					entryNode.SetAttribute("Source", entry->GetID());
				}
				entryNode.SetAttribute("Destination", entry->GetDestination());
	
				if (!entry->IsDefaultPriority())
				{
					entryNode.SetAttribute("Priority", entry->GetPriority());
				}
	
				if (m_AsProject && folderEntry && !folderEntry->GetFiles().empty())
				{
					for (const FolderItemElement& fileEntry: folderEntry->GetFiles())
					{
						KxXMLNode fileEntryNode = entryNode.NewElement("Entry");
						fileEntryNode.SetValue(fileEntry.GetDestination());
						fileEntryNode.SetAttribute("Source", fileEntry.GetSource());
					}
				}
			}
		}
	}
	void NativeSerializer::WriteRequirements(KxXMLNode& baseNode)
	{
		const RequirementsSection& requirements = m_ProjectSave->GetRequirements();
		KxXMLNode requirementsNode = baseNode.NewElement("Requirements");
		if (!requirements.IsDefaultGroupEmpty())
		{
			KAux::SaveStringArray(requirements.GetDefaultGroup(), requirementsNode.NewElement("DefaultGroups"));
		}
	
		if (!requirements.GetGroups().empty())
		{
			KxXMLNode groupsArrayNode = requirementsNode.NewElement("Groups");
			for (const auto& group: requirements.GetGroups())
			{
				KxXMLNode requirementsGroupNode = groupsArrayNode.NewElement("Group");
				requirementsGroupNode.SetAttribute("ID", group->GetID());
				requirementsGroupNode.SetAttribute("Operator", ModPackageProject::OperatorToString(group->GetOperator()));
	
				if (!group->GetEntries().empty())
				{
					for (const auto& entry: group->GetEntries())
					{
						KxXMLNode entryNode = requirementsGroupNode.NewElement("Entry");
						if (!entry->IsEmptyID())
						{
							entryNode.SetAttribute("ID", entry->RawGetID());
						}
						entryNode.SetAttribute("Type", requirements.TypeDescriptorToString(entry->GetTypeDescriptor()));
	
						// Name
						entryNode.NewElement("Name").SetValue(entry->RawGetName());
	
						// Object
						KxXMLNode objectNode = entryNode.NewElement("Object");
						objectNode.SetValue(entry->GetObject());
						objectNode.SetAttribute("Function", requirements.ObjectFunctionToString(entry->GetObjectFunction()));
	
						// Version
						KxXMLNode versionNode = entryNode.NewElement("Version");
						versionNode.SetValue(entry->GetRequiredVersion());
						versionNode.SetAttribute("Function", ModPackageProject::OperatorToString(entry->GetRVFunction()));
	
						// Description
						if (!entry->GetDescription().IsEmpty())
						{
							entryNode.NewElement("Description").SetValue(entry->GetDescription());
						}
					}
				}
			}
		}
	}
	void NativeSerializer::WriteComponents(KxXMLNode& baseNode)
	{
		KxXMLNode componentsNode = baseNode.NewElement("Components");
		const ComponentsSection& components = m_ProjectSave->GetComponents();
	
		// Write required files
		if (!components.GetRequiredFileData().empty())
		{
			KAux::SaveStringArray(components.GetRequiredFileData(), componentsNode.NewElement("RequiredFiles"));
		}
	
		// Write steps
		if (!components.GetSteps().empty())
		{
			KxXMLNode stepsArrayNode = componentsNode.NewElement("Steps");
			for (const auto& step: components.GetSteps())
			{
				/* Header */
				KxXMLNode stepNode = stepsArrayNode.NewElement("Step");
				if (!step->IsEmptyName())
				{
					stepNode.SetAttribute("Name", step->GetName());
				}
	
				/* Step conditions */
				if (step->GetConditionGroup().HasConditions())
				{
					WriteConditionGroup(step->GetConditionGroup(), stepNode.NewElement("ConditionGroup"));
				}
	
				/* Groups */
				if (!step->GetGroups().empty())
				{
					KxXMLNode groupsArrayNode = stepNode.NewElement("Groups");
					for (const auto& group: step->GetGroups())
					{
						KxXMLNode groupNode = groupsArrayNode.NewElement("Group");
						if (!group->IsEmptyName())
						{
							groupNode.SetAttribute("Name", group->GetName());
						}
						groupNode.SetAttribute("SelectionMode", components.SelectionModeToString(group->GetSelectionMode()));
	
						// Group entries
						if (!group->GetEntries().empty())
						{
							KxXMLNode tEntriesArrayNode = groupNode.NewElement("Entries");
							for (const auto& entry: group->GetEntries())
							{
								KxXMLNode entryNode = tEntriesArrayNode.NewElement("Entry");
	
								// Name is required
								entryNode.NewElement("Name").SetValue(entry->GetName());
	
								// Image
								if (!entry->GetImage().IsEmpty())
								{
									wxString image = m_AsProject ? entry->GetImage() : PathNameToPackage(entry->GetImage(), ContentType::Images);
									entryNode.NewElement("Image").SetAttribute("Path", image);
								}
	
								// Description
								if (!entry->GetDescription().IsEmpty())
								{
									entryNode.NewElement("Description").SetValue(entry->GetDescription());
								}
	
								// Type descriptor
								KxXMLNode typeDescriptorNode = entryNode.NewElement("TypeDescriptor");
								typeDescriptorNode.SetAttribute("DefaultValue", components.TypeDescriptorToString(entry->GetTDDefaultValue()));
								if (entry->GetTDConditionalValue() != TypeDescriptor::Invalid)
								{
									typeDescriptorNode.SetAttribute("ConditionalValue", components.TypeDescriptorToString(entry->GetTDConditionalValue()));
								}
	
								if (entry->GetTDConditionGroup().HasConditions())
								{
									WriteConditionGroup(entry->GetTDConditionGroup(), typeDescriptorNode.NewElement("Conditions"));
								}
	
								if (!entry->GetFileData().empty())
								{
									KAux::SaveStringArray(entry->GetFileData(), entryNode.NewElement("Files"));
								}
	
								if (!entry->GetRequirements().empty())
								{
									KAux::SaveStringArray(entry->GetRequirements(), entryNode.NewElement("Requirements"));
								}
	
								if (entry->GetConditionalFlags().HasFlags())
								{
									WriteCondition(entry->GetConditionalFlags(), entryNode.NewElement("ConditionalFlags"), false);
								}
							}
						}
					}
				}
			}
		}
	
		auto WriteConditionalSteps = [&componentsNode](const ConditionalComponentStep::Vector& steps, const wxString& sRootNodeName, const wxString& sNodeName)
		{
			if (!steps.empty())
			{
				KxXMLNode stepsArrayNode = componentsNode.NewElement(sRootNodeName);
				for (const auto& step: steps)
				{
					/* Header */
					KxXMLNode setNode = stepsArrayNode.NewElement("Step");
					if (step->GetConditionGroup().HasConditions())
					{
						WriteConditionGroup(step->GetConditionGroup(), setNode.NewElement("Conditions"));
					}
	
					/* Entries */
					if (!step->GetEntries().empty())
					{
						KAux::SaveStringArray(step->GetEntries(), setNode.NewElement(sNodeName));
					}
				}
			}
		};
		WriteConditionalSteps(components.GetConditionalSteps(), "ConditionalSteps", "Files");
	}
	
	void NativeSerializer::Serialize(const ModPackageProject* project)
	{
		m_ProjectSave = project;
		m_XML.Load(wxEmptyString);
	
		KxXMLNode baseNode = WriteBase();
		WriteConfig(baseNode);
		WriteInfo(baseNode);
		WriteFiles(baseNode);
		WriteInterface(baseNode);
		WriteRequirements(baseNode);
		WriteComponents(baseNode);
	
		m_Data = m_XML.Save();
	}
	void NativeSerializer::Structurize(ModPackageProject* project)
	{
		m_ProjectLoad = project;
		m_XML.Load(m_Data);
	
		ReadBase();
		ReadConfig();
		ReadInfo();
		ReadFiles();
		ReadInterface();
		ReadRequirements();
		ReadComponents();
	}
}
