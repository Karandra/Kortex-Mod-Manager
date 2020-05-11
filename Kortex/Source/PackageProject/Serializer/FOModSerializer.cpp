#include "stdafx.h"
#include "FOModSerializer.h"
#include "PackageProject/ModPackageProject.h"
#include "PackageProject/ConfigSection.h"
#include "PackageProject/InfoSection.h"
#include "PackageProject/InterfaceSection.h"
#include "PackageProject/FileDataSection.h"
#include "PackageProject/RequirementsSection.h"
#include "PackageProject/ComponentsSection.h"
#include "ModPackages/IPackageManager.h"

#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/PackageManager.hpp>

#include "Network/ModNetwork/Nexus.h"
#include "Network/ModNetwork/LoversLab.h"
#include "Network/ModNetwork/TESALL.h"
#include "Utility/UnsortedUnique.h"
#include <KxFramework/KxString.h>

namespace Kortex::PackageProject
{
	using WriteEmpty = KxIXDocumentNode::WriteEmpty;

	namespace
	{
		wxString ToFOModOperator(Operator value)
		{
			return value == Operator::Or ? wxS("Or") : wxS("And");
		}
		Operator FromFOModOperator(const wxString& value)
		{
			return value == wxS("Or") ? Operator::Or : Operator::And;
		}
	
		void WriteCondition(const Condition& condition, KxXMLNode& conditionNode)
		{
			for (const FlagItem& flag: condition.GetFlags())
			{
				if (flag.HasName())
				{
					KxXMLNode entryNode = conditionNode.NewElement("flagDependency");
					entryNode.SetAttribute("flag", flag.GetName());
					entryNode.SetAttribute("value", flag.GetValue());
				}
			}
		}
		void WriteConditionGroup(const ConditionGroup& conditionGroup, KxXMLNode& groupNode)
		{
			groupNode.SetAttribute("operator", ToFOModOperator(conditionGroup.GetOperator()));
	
			const Condition::Vector& conditions = conditionGroup.GetConditions();
			if (conditions.size() == 1)
			{
				if (conditions.front().HasFlags())
				{
					WriteCondition(conditions.front(), groupNode);
				}
			}
			else
			{
				for (const Condition& condition: conditions)
				{
					if (condition.HasFlags())
					{
						WriteCondition(condition, groupNode.NewElement("dependencies"));
					}
				}
			}
		}
	
		void ReadAssignedFlags(Condition& condition, const KxXMLNode& conditionNode)
		{
			for (KxXMLNode node = conditionNode.GetFirstChildElement("flag"); node.IsOK(); node = node.GetNextSiblingElement("flag"))
			{
				condition.GetFlags().emplace_back(node.GetValue(), node.GetAttribute("name"));
			}
		}
		void WriteAssignedFlags(const Condition& condition, KxXMLNode& conditionNode)
		{
			conditionNode.SetAttribute("operator", ToFOModOperator(condition.GetOperator()));
			for (const FlagItem& flag : condition.GetFlags())
			{
				if (flag.HasName())
				{
					KxXMLNode entryNode = conditionNode.NewElement("flag");
					entryNode.SetValue(flag.GetValue());
					entryNode.SetAttribute("name", flag.GetName());
				}
			}
		}
	
		void ReadCompositeDependenciesAux(ModPackageProject& project,
										  const KxXMLNode& dependenciesNode,
										  ConditionGroup* conditionGroup,
										  ComponentItem* componentEntry,
										  bool alwaysCreateReqGroup,
										  const wxString& createdReqGroupID,
										  RequirementGroup*& requirementsGroup
		)
		{
			Operator operatorType = FromFOModOperator(dependenciesNode.GetAttribute("operator"));
	
			auto CreateOrGetRequirementsGroup = [&project, &createdReqGroupID, &requirementsGroup, operatorType]()
			{
				if (!requirementsGroup)
				{
					requirementsGroup = project.GetRequirements().GetGroups().emplace_back(new RequirementGroup()).get();
					requirementsGroup->SetID(createdReqGroupID);
					requirementsGroup->SetOperator(operatorType);
				}
				return requirementsGroup;
			};
			if (alwaysCreateReqGroup)
			{
				CreateOrGetRequirementsGroup();
			}
	
			for (KxXMLNode depNode = dependenciesNode.GetFirstChildElement(); depNode.IsOK(); depNode = depNode.GetNextSiblingElement())
			{
				std::unique_ptr<RequirementItem> reqEntry;
	
				wxString name = depNode.GetName();
				if (name == "fileDependency")
				{
					reqEntry = std::make_unique<RequirementItem>();
					reqEntry->SetObject(depNode.GetAttribute("file"));
	
					// FOMod support only these three required states
					wxString state = depNode.GetAttribute("state");
					if (state == "Active")
					{
						reqEntry->SetObjectFunction(ObjectFunction::PluginActive);
					}
					else if (state == "Inactive")
					{
						reqEntry->SetObjectFunction(ObjectFunction::PluginInactive);
					}
					else if (state == "Missing")
					{
						reqEntry->SetObjectFunction(ObjectFunction::FileNotExist);
					}
					else
					{
						reqEntry->SetObjectFunction(ObjectFunction::None);
					}
				}
				else if (name == "gameDependency")
				{
					reqEntry = std::make_unique<RequirementItem>();
	
					// Copy std requirement for current game and set required version from FOMod
					const RequirementItem* stdEntry = IPackageManager::GetInstance()->FindStdReqirement(IGameInstance::GetActive()->GetGameID());
	
					// This check probably redundant, but just in case
					if (stdEntry)
					{
						*reqEntry = *stdEntry;
					}
					else
					{
						reqEntry->SetObjectFunction(ObjectFunction::FileExist);
					}
					reqEntry->SetRequiredVersion(depNode.GetAttribute("version"));
				}
				else if (name == "foseDependency")
				{
					// Although it's named 'fose' I will interpret this as generic Script Extender requirement.
					// What else can I do? Check game ID and continue only if it's Fallout 3?
					reqEntry = std::make_unique<RequirementItem>();
	
					// There may be no Script Extender
					if (auto se = IPackageManager::GetInstance()->TryGetComponent<PackageDesigner::IWithScriptExtender>())
					{
						*reqEntry = se->GetEntry();
					}
					else
					{
						// No SE, fill with something meaningful
						reqEntry->SetName(IGameInstance::GetActive()->GetGameShortName() + " Script Extender");
						reqEntry->SetObjectFunction(ObjectFunction::FileExist);
					}
				}
				else if (name == "flagDependency")
				{
					// This is an equivalent of native 'Flag' attribute.
					if (conditionGroup)
					{
						wxString name = depNode.GetAttribute("flag");
						wxString value = depNode.GetAttribute("value");
						conditionGroup->GetOrCreateFirstCondition().GetFlags().emplace_back(value, name);
					}
				}
				else if (name == "fommDependency")
				{
					// This could be interpreted as mod manager version.
					// Native format doesn't currently support that. It has a 'FormatVersion' attribute,
					// but versions used in FOMod will surely be different from versions
					// of Kortex's install engine. So ignore this for now.
				}
				else
				{
					// There also can be 'dependencies' element which can include another level of this structure.
					// This seems wrong and I will not process this.
				}
	
				// If requirement entry has been created, force creation of a requirements group and add entry there
				if (reqEntry)
				{
					// Create requirements group if needed and it haven't created earlier
					CreateOrGetRequirementsGroup();
	
					// If no name assigned to this entry (which is always the case right now),
					// extract name from the requirement object file path.
					if (reqEntry->IsEmptyName())
					{
						reqEntry->SetName(reqEntry->GetObject().AfterLast('\\').BeforeLast('.'));
					}
	
					// If still empty, use address as the name
					if (reqEntry->IsEmptyName())
					{
						reqEntry->SetName(KxString::Format("FOModReq::0x%1", reqEntry.get()));
					}
	
					// Try change type to system. Most likely will fail.
					reqEntry->SetType(ReqType::System);
	
					// Add the entry to its group
					requirementsGroup->GetItems().emplace_back(std::move(reqEntry));
				}
			}
	
			// Set this requirements group as requirement to conditions group
			if (conditionGroup && requirementsGroup)
			{
				conditionGroup->GetOrCreateFirstCondition().GetFlags().emplace_back("true", requirementsGroup->GetFlagName());
			}
	
			// Link this requirements group to provided component
			if (componentEntry && requirementsGroup)
			{
				componentEntry->GetRequirements().emplace_back(requirementsGroup->GetID());
			}
		}
		RequirementGroup* ReadCompositeDependencies(ModPackageProject& project,
														 const KxXMLNode& dependenciesNode,
														 ConditionGroup* conditionGroup,
														 ComponentItem* componentsEntry,
														 bool alwaysCreateReqGroup = false,
														 const wxString& createdReqGroupID = wxEmptyString
		)
		{
			// If we have child dependencies read them all to condition group.
			// Else read them from this node.
			RequirementGroup* requirementsGroup = nullptr;
			KxXMLNode childDependenciesNode = dependenciesNode.GetFirstChildElement("dependencies");
			if (childDependenciesNode.IsOK())
			{
				for (KxXMLNode node = childDependenciesNode; node.IsOK(); node = node.GetNextSiblingElement("dependencies"))
				{
					ReadCompositeDependenciesAux(project, node, conditionGroup, componentsEntry, alwaysCreateReqGroup, createdReqGroupID, requirementsGroup);
				}
			}
			else
			{
				ReadCompositeDependenciesAux(project, dependenciesNode, conditionGroup, componentsEntry, alwaysCreateReqGroup, createdReqGroupID, requirementsGroup);
			}
			return requirementsGroup;
		}
	
		template<class T>
		void SortEntries(T& array, const wxString& order)
		{
			if (order != wxS("Explicit"))
			{
				const bool isLess = order == wxS("Ascending");
				std::sort(array.begin(), array.end(), [isLess](const auto& v1, const auto& v2)
				{
					return isLess ? v1->GetName() < v2->GetName() : v1->GetName() > v2->GetName();
				});
			}
		}
		
		template<class T>
		bool WriteSite(const ModSourceStore& modSourceStore, KxXMLNode& node)
		{
			if (const ModSourceItem* item = modSourceStore.GetItem<T>())
			{
				node.SetValue(item->GetURI().BuildUnescapedURI(), WriteEmpty::Never);
				return true;
			}
			return false;
		};
	}
}

namespace Kortex::PackageProject
{
	wxString FOModSerializer::GetDataFolderName(bool withSeparator) const
	{
		wxString folder;
		if (m_IsMorrowind)
		{
			folder = wxS("DataFiles");
		}
		else if (m_HasDataFolderAsRoot)
		{
			folder = wxS("Data");
		}
	
		if (!folder.IsEmpty() && withSeparator)
		{
			folder.Append(wxS('\\'));
		}
		return folder;
	}
	wxString FOModSerializer::MakeProjectPath(const wxString& path) const
	{
		if (!path.IsEmpty())
		{
			wxString newPath = !m_ProjectFolder.IsEmpty() ? m_ProjectFolder + '\\' + path : path;
			if (!m_EffectiveArchiveRoot.IsEmpty())
			{
				newPath = m_EffectiveArchiveRoot + '\\' + newPath;
			}
			return newPath;
		}
		return wxEmptyString;
	}
	SelectionMode FOModSerializer::ConvertSelectionMode(const wxString& mode) const
	{
		// Remove 'Select' (first 6 chars) from string and use internal conversion function.
		return ComponentsSection::StringToSelectionMode(wxString(mode).Remove(0, 6));
	}
	wxString FOModSerializer::ConvertSelectionMode(SelectionMode mode) const
	{
		// Append internal conversion function result to 'Select'
		return wxS("Select") + ComponentsSection::SelectionModeToString(mode);
	}
	
	/* Structurize */
	void FOModSerializer::ReadInfo()
	{
		KxXMLNode infoNode = m_XML.QueryElement("fomod");
		if (infoNode.IsOK())
		{
			InfoSection& info = m_ProjectLoad->GetInfo();
	
			// Basic info
			info.SetName(infoNode.GetFirstChildElement("Name").GetValue());
			info.SetVersion(infoNode.GetFirstChildElement("Version").GetValue());
			info.SetAuthor(infoNode.GetFirstChildElement("Author").GetValue());
			info.SetDescription(ConvertBBCode(KxString::Trim(infoNode.GetFirstChildElement("Description").GetValue(), true, true)));
	
			// Web-site
			ModSourceStore& modSourceStore = info.GetModSourceStore();
	
			ModID nexusID = infoNode.GetFirstChildElement("Id").GetValueInt(ModID::GetInvalidValue());
			if (nexusID.HasValue())
			{
				modSourceStore.TryAddWith<NetworkManager::NexusModNetwork>(nexusID);
			}
	
			wxString siteURL = infoNode.GetFirstChildElement("Website").GetValue();
			if (!siteURL.IsEmpty())
			{
				auto AddAsGenericSite = [&modSourceStore, &siteURL](const wxString& siteName)
				{
					modSourceStore.TryAddWith(siteName.AfterLast('.'), siteURL);
				};
	
				wxString siteName;
				ModSourceItem webSite = TryParseWebSite(siteURL, &siteName);
				if (webSite.IsOK())
				{
					// Site for Nexus already retrieved, so add as generic
					IModNetwork* modNetwork = nullptr;
					if (webSite.TryGetModNetwork(modNetwork) && modNetwork == NetworkManager::NexusModNetwork::GetInstance())
					{
						AddAsGenericSite(siteName);
					}
					else
					{
						modSourceStore.AssignItem(std::move(webSite));
					}
				}
				else
				{
					AddAsGenericSite(siteName);
				}
			}
	
			// Load tags
			ModTagStore& tagStore = info.GetTagStore();
			for (KxXMLNode node = infoNode.GetFirstChildElement("Groups").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				auto tag = IModTagManager::GetInstance()->NewTag();
				tag->SetID(node.GetValue());
				tagStore.AddTag(*tag);
			}
		}
	}
	
	void FOModSerializer::ReadInstallSteps()
	{
		InfoSection& info = m_ProjectLoad->GetInfo();
		InterfaceSection& interfaceConfig = m_ProjectLoad->GetInterface();
		RequirementsSection& requirements = m_ProjectLoad->GetRequirements();
		ComponentsSection& components = m_ProjectLoad->GetComponents();
	
		KxXMLNode configRootNode = m_XML.QueryElement("config");
		if (configRootNode.IsOK())
		{
			/* Header, sort of */
			KxXMLNode moduleNameNode = configRootNode.GetFirstChildElement("moduleName");
			if (moduleNameNode.IsOK())
			{
				// Try to decide whether to use module name as ID or not
				wxString moduleName = moduleNameNode.GetValue();
				if (!moduleName.IsEmpty())
				{
					// Use it as just name if it's not specified in Info.xml
					if (info.GetName().IsEmpty())
					{
						info.SetName(moduleName);
					}
					else if (info.GetName() != moduleName)
					{
						// Set module name as ID only if it's different from mod name (in Info.xml)
						m_ProjectLoad->SetModID(moduleName);
					}
				}
	
				// Read title customization
				TitleConfig& titleConfig = interfaceConfig.GetTitleConfig();
	
				int64_t color = moduleNameNode.GetAttributeInt("colour", -1);
				if (color != -1)
				{
					titleConfig.SetColor(KxColor::FromRGBA(color));
				}
	
				wxString alignment = moduleNameNode.GetAttribute("position");
				if (alignment == "Left")
				{
					titleConfig.SetAlignment(wxALIGN_LEFT);
				}
				else if (alignment == "Right" || alignment == "RightOfImage")
				{
					// Mode 'RightOfImage' can not be described in 'wxAlignment' terms.
					// And title placement doesn't really matter anyway.
					titleConfig.SetAlignment(wxALIGN_RIGHT);
				}
			}
	
			// Add required files to project and link them to components' required files
			for (const FileItem* fileItem: ReadFileData(configRootNode.GetFirstChildElement("requiredInstallFiles")))
			{
				components.GetRequiredFileData().emplace_back(fileItem->GetID());
			}
	
			// Header image
			KxXMLNode headerImageNode = configRootNode.GetFirstChildElement("moduleImage");
			if (headerImageNode.IsOK())
			{
				ImageItem& entry = interfaceConfig.GetImages().emplace_back();
				entry.SetPath(MakeProjectPath(headerImageNode.GetAttribute("path")));
				entry.SetVisible(headerImageNode.GetAttributeBool("showImage", true));
				entry.SetFadeEnabled(headerImageNode.GetAttributeBool("showFade", entry.IsVisible()));
				entry.SetSize(wxSize(wxDefaultCoord, headerImageNode.GetAttributeInt("height", wxDefaultCoord)));
	
				interfaceConfig.SetHeaderImage(entry.GetPath());
			}
	
			// Main requirements
			KxXMLNode moduleReqsNode = configRootNode.GetFirstChildElement("moduleDependencies").GetFirstChildElement("dependencies");
			if (!moduleReqsNode.IsOK())
			{
				moduleReqsNode = configRootNode.GetFirstChildElement("moduleDependencies");
			}
			RequirementGroup* mainReqsGroup = ReadCompositeDependencies(*m_ProjectLoad, moduleReqsNode, nullptr, nullptr, true, "Main");
			if (mainReqsGroup)
			{
				// If main requirements group is empty - add current game with no required version
				if (mainReqsGroup->GetItems().empty())
				{
					RequirementItem* entry = mainReqsGroup->GetItems().emplace_back(std::make_unique<RequirementItem>()).get();
					entry->SetID(IGameInstance::GetActive()->GetGameID());
					entry->ConformToType();
				}
				requirements.GetDefaultGroup().push_back(mainReqsGroup->GetID());
			}
	
			/* Install steps */
			auto& steps = components.GetSteps();
	
			KxXMLNode tInstallStepsArrayNode = configRootNode.GetFirstChildElement("installSteps");
			wxString stepsOrder = tInstallStepsArrayNode.GetAttribute("order");
			for (KxXMLNode stepNode = tInstallStepsArrayNode.GetFirstChildElement("installStep"); stepNode.IsOK(); stepNode = stepNode.GetNextSiblingElement("installStep"))
			{
				ComponentStep* step = steps.emplace_back(std::make_unique<ComponentStep>()).get();
				step->SetName(stepNode.GetAttribute("name"));
	
				// Step conditions
				{
					KxXMLNode stepConditionsNode = stepNode.GetFirstChildElement("visible").GetFirstChildElement("dependencies");
					if (!stepConditionsNode.IsOK())
					{
						stepConditionsNode = stepNode.GetFirstChildElement("visible");
					}
					ReadCompositeDependencies(*m_ProjectLoad, stepConditionsNode, &step->GetConditionGroup(), nullptr, false, step->GetName());
				}
	
				KxXMLNode optionalFileGroupsNode = stepNode.GetFirstChildElement("optionalFileGroups");
				if (optionalFileGroupsNode.IsOK())
				{
					for (KxXMLNode groupNode = optionalFileGroupsNode.GetFirstChildElement("group"); groupNode.IsOK(); groupNode = groupNode.GetNextSiblingElement("group"))
					{
						ComponentGroup* group = step->GetGroups().emplace_back(std::make_unique<ComponentGroup>()).get();
						group->SetName(groupNode.GetAttribute("name"));
						group->SetSelectionMode(ConvertSelectionMode(groupNode.GetAttribute("type")));
	
						KxXMLNode pluginsArrayNode = groupNode.GetFirstChildElement("plugins");
						wxString pluginsOrder = pluginsArrayNode.GetAttribute("order");
	
						for (KxXMLNode pluginNode = pluginsArrayNode.GetFirstChildElement("plugin"); pluginNode.IsOK(); pluginNode = pluginNode.GetNextSiblingElement("plugin"))
						{
							ComponentItem* entry = group->GetItems().emplace_back(std::make_unique<ComponentItem>()).get();
							entry->SetName(pluginNode.GetAttribute("name"));
	
							// Description
							wxString description = ConvertBBCode(KxString::Trim(pluginNode.GetFirstChildElement("description").GetValue(), true, true));
							entry->SetDescription(description);
	
							// Image
							wxString pluginImage = MakeProjectPath(pluginNode.GetFirstChildElement("image").GetAttribute("path"));
							if (!pluginImage.IsEmpty())
							{
								interfaceConfig.GetImages().emplace_back(ImageItem(pluginImage, wxEmptyString, true));
								entry->SetImage(pluginImage);
							}
	
							// Type descriptor (they are identical to my own since they was ported from FOMod)
							KxXMLNode typeDescriptorNode = pluginNode.GetFirstChildElement("typeDescriptor").GetFirstChildElement();
							wxString typeDescriptorNodeName = typeDescriptorNode.GetName();
							if (typeDescriptorNodeName == "type")
							{
								// Simple variant
								wxString typeDescriptor = typeDescriptorNode.GetAttribute("name");
								entry->SetTDDefaultValue(ComponentsSection::StringToTypeDescriptor(typeDescriptor));
							}
							else if (typeDescriptorNodeName == "dependencyType")
							{
								// Dependencies check variant
	
								// By scheme there may be multiple 'pattern' nodes inside 'patterns' but I haven't seen any FOMod that uses such configuration
								// nor there is any sense in doing this.
								KxXMLNode node = typeDescriptorNode.GetFirstChildElement("patterns").GetFirstChildElement("pattern");
								entry->SetTDDefaultValue(ComponentsSection::StringToTypeDescriptor(typeDescriptorNode.GetFirstChildElement("defaultType").GetAttribute("name")));
								entry->SetTDConditionalValue(ComponentsSection::StringToTypeDescriptor(node.GetFirstChildElement("type").GetAttribute("name")));
	
								wxString reqGroupID = KxString::Format("%1::%2::%3", step->GetName(), group->GetName(), entry->GetName());
								ReadCompositeDependencies(*m_ProjectLoad, node.GetFirstChildElement("dependencies"), &entry->GetTDConditionGroup(), entry, false, reqGroupID);
							}
	
							// Assigned flags
							ReadAssignedFlags(entry->GetConditionFlags(), pluginNode.GetFirstChildElement("conditionFlags"));
	
							// Files
							ReadFileData(pluginNode.GetFirstChildElement("files"), entry);
						}
	
						// Sort entries in set
						SortEntries(group->GetItems(), pluginsOrder);
					}
				}
			}
	
			// Sort steps
			SortEntries(components.GetSteps(), stepsOrder);
	
			// Load conditional steps
			ReadConditionalSteps(configRootNode.GetFirstChildElement("conditionalFileInstalls").GetFirstChildElement("patterns"));
	
			// Since FOMod doesn't have conception of main image (or I just don't know about it),
			// use first available image as main image.
			if (!interfaceConfig.GetImages().empty())
			{
				interfaceConfig.SetMainImage(interfaceConfig.GetImages().front().GetPath());
			}
		}
	}
	void FOModSerializer::ReadConditionalSteps(const KxXMLNode& stepsArrayNode)
	{
		RequirementsSection& requirements = m_ProjectLoad->GetRequirements();
		ComponentsSection& components = m_ProjectLoad->GetComponents();
		auto& conditionalSteps = components.GetConditionalSteps();
	
		size_t index = 1;
		for (KxXMLNode stepNode = stepsArrayNode.GetFirstChildElement("pattern"); stepNode.IsOK(); stepNode = stepNode.GetNextSiblingElement("pattern"))
		{
			ConditionalComponentStep* step = conditionalSteps.emplace_back(std::make_unique<ConditionalComponentStep>()).get();
	
			// Files
			for (const FileItem* fileItem: ReadFileData(stepNode.GetFirstChildElement("files")))
			{
				step->GetItems().emplace_back(fileItem->GetID());
			}
	
			// Conditions
			RequirementGroup* reqSet = ReadCompositeDependencies(*m_ProjectLoad, stepNode.GetFirstChildElement("dependencies"), &step->GetConditionGroup(), nullptr);
			if (reqSet)
			{
				reqSet->SetID(KxString::Format("ConditionalStep#%1", index));
				step->GetConditionGroup().GetOrCreateFirstCondition().GetFlags().emplace_back(FlagItem("true", reqSet->GetFlagName()));
			}
		}
	}
	std::vector<FileItem*> FOModSerializer::ReadFileData(const KxXMLNode& filesArrayNode, ComponentItem* entry)
	{
		std::vector<std::pair<std::unique_ptr<FileItem>, int64_t>> priorityList;
		if (filesArrayNode.IsOK())
		{
			for (KxXMLNode fileDataNode = filesArrayNode.GetFirstChildElement(); fileDataNode.IsOK(); fileDataNode = fileDataNode.GetNextSiblingElement())
			{
				std::unique_ptr<FileItem> fileEntry;
				if (fileDataNode.GetName() == wxS("folder"))
				{
					fileEntry = std::make_unique<FolderItem>();
				}
				else
				{
					fileEntry = std::make_unique<FileItem>();
				}
	
				wxString source = fileDataNode.GetAttribute("source");
				fileEntry->SetID(source);
				fileEntry->SetSource(MakeProjectPath(source));
				fileEntry->SetPriority(fileDataNode.GetAttributeInt("priority", FileDataSection::ms_DefaultPriority));
	
				wxString destination = fileDataNode.GetAttribute("destination");
				fileEntry->SetDestination(GetDataFolderName(true) + (!destination.IsEmpty() ? destination : wxEmptyString));

				// Decide whether to add this item to required files or not
				bool shouldAlwaysInstall = fileDataNode.GetAttributeBool("alwaysInstall", false);
				bool shouldInstallIfUsable = fileDataNode.GetAttributeBool("installIfUsable", false);
				if (shouldAlwaysInstall || (shouldInstallIfUsable && entry->GetTDDefaultValue() != TypeDescriptor::NotUsable))
				{
					m_ProjectLoad->GetComponents().GetRequiredFileData().emplace_back(fileEntry->GetID());
				}

				priorityList.emplace_back(std::move(fileEntry), fileDataNode.GetAttributeInt("priority", std::numeric_limits<int64_t>::max()));
			}
	
			// Sort by priority
			std::sort(priorityList.begin(), priorityList.end(), [](const auto& left, const auto& right)
			{
				return left.second < right.second;
			});
			
			// Add to project and link these files to component if needed
			std::vector<FileItem*> refVector;
			for (auto& [fileItem, filePriority]: priorityList)
			{
				refVector.push_back(fileItem.get());

				if (entry)
				{
					entry->GetFileData().emplace_back(fileItem->GetID());
				}
				m_ProjectLoad->GetFileData().AddFile(std::move(fileItem));
			}
			return refVector;
		}
		return {};
	}
	void FOModSerializer::UniqueFileData()
	{
		auto& files = m_ProjectLoad->GetFileData().GetItems();
		auto it = std::unique(files.begin(), files.end(), [](const auto& v1, const auto& v2)
		{
			return v1->GetID() == v2->GetID();
		});
		files.erase(it, files.end());
	}
	void FOModSerializer::UniqueImages()
	{
		ImageItem::Vector& images = m_ProjectLoad->GetInterface().GetImages();
		auto it = Utility::UnsortedUnique(images.begin(), images.end(), [](const ImageItem& v1, const ImageItem& v2)
		{
			return v1.GetPath() == v2.GetPath();
		},
		[](const ImageItem& v1, const ImageItem& v2)
		{
			return v1.GetPath() < v2.GetPath();
		});
		images.erase(it, images.end());
	}
	
	/* Serialize */
	void FOModSerializer::WriteInfo()
	{
		const InfoSection& info = m_ProjectSave->GetInfo();
		KxXMLNode infoNode = m_XML.NewElement("fomod");
	
		infoNode.NewElement("Name").SetValue(info.GetName());
		infoNode.NewElement("Author").SetValue(info.GetAuthor());
	
		KxXMLNode versionNode = infoNode.NewElement("Version");
		versionNode.SetValue(info.GetVersion());
		versionNode.SetAttribute("MachineVersion", "5.0");
	
		if (!info.GetDescription().IsEmpty())
		{
			infoNode.NewElement("Description").SetValue(info.GetDescription());
		}
	
		// Sites
		WriteSites(infoNode, infoNode.NewElement("Website"));
	
		// Tags
		KxXMLNode tagsNode = infoNode.NewElement("Groups");
		info.GetTagStore().Visit([&tagsNode](const IModTag& tag)
		{
			tagsNode.NewElement("element").SetValue(tag.GetID());
			return true;
		});
	}
	void FOModSerializer::WriteSites(KxXMLNode& infoNode, KxXMLNode& sitesNode)
	{
		using namespace NetworkManager;

		// FOMod supports only one web-site and field for site ID, so I need to decide which one to write.
		// The order will be: Nexus (as ID) -> LoversLab -> TESALL -> other (if any)
	
		const ModSourceStore& modSourceStore = m_ProjectSave->GetInfo().GetModSourceStore();
	
		// Write Nexus to 'Id'
		if (const ModSourceItem* nexusItem = modSourceStore.GetItem(NexusModNetwork::GetInstance()->GetName()))
		{
			infoNode.NewElement("Id").SetValue(nexusItem->GetModInfo().GetModID().GetValue());
		}
	
		if (!(WriteSite<LoversLabModNetwork>(modSourceStore, sitesNode) || WriteSite<TESALLModNetwork>(modSourceStore, sitesNode)))
		{
			// Write first one from store
			modSourceStore.Visit([&sitesNode](const ModSourceItem& item)
			{
				sitesNode.SetValue(item.GetURI().BuildUnescapedURI());
				return false;
			});
		}
	
		// Ignore all others sites
	}
	
	void FOModSerializer::WriteInstallSteps()
	{
		using namespace Application;

		const InterfaceSection& interfaceConfig = m_ProjectSave->GetInterface();
		const RequirementsSection& requirements = m_ProjectSave->GetRequirements();
		const ComponentsSection& components = m_ProjectSave->GetComponents();
	
		KxXMLNode configRootNode = m_XML.NewElement("config");
	
		// Write XML-Schema
		if (GetGlobalOptionOf<IPackageManager>(OName::FOMod).GetAttributeBool(OName::UseHTTPSForXMLScheme, true))
		{
			configRootNode.SetAttribute("xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");
			configRootNode.SetAttribute("xsi:noNamespaceSchemaLocation", "https://qconsulting.ca/fo3/ModConfig5.0.xsd");
		}
		else
		{
			configRootNode.SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
			configRootNode.SetAttribute("xsi:noNamespaceSchemaLocation", "http://qconsulting.ca/fo3/ModConfig5.0.xsd");
		}
	
		// Write name
		KxXMLNode moduleNameNode = configRootNode.NewElement("moduleName");
		moduleNameNode.SetValue(m_ProjectSave->GetModID());
	
		// Write title customization
		const TitleConfig& titleConfig = interfaceConfig.GetTitleConfig();
		if (titleConfig.HasAlignment())
		{
			if (titleConfig.GetAlignment() == wxALIGN_LEFT)
			{
				moduleNameNode.SetAttribute("position", "Left");
			}
			else if (titleConfig.GetAlignment() == wxALIGN_RIGHT)
			{
				moduleNameNode.SetAttribute("position", "Right");
			}
		}
		if (titleConfig.HasColor())
		{
			wxString colorValue = titleConfig.GetColor().ToString(KxColor::C2S::HTML).AfterFirst('#');
			if (!colorValue.IsEmpty())
			{
				// It's "colour" in FOMod. Yes, British spelling.
				moduleNameNode.SetAttribute("colour", colorValue);
			}
		}
	
		// Write header image
		if (const ImageItem* headerImageEntry = interfaceConfig.GetHeaderItem())
		{
			KxXMLNode node = configRootNode.NewElement("moduleImage");
			node.SetAttribute("path", PathNameToPackage(headerImageEntry->GetPath(), ContentType::Images));
			node.SetAttribute("showImage", headerImageEntry->IsVisible() ? "true" : "false");
			node.SetAttribute("showFade", headerImageEntry->IsFadeEnabled() ? "true" : "false");
			
			int height = headerImageEntry->GetSize().GetHeight();
			if (height != wxDefaultCoord)
			{
				node.SetAttribute("height", height);
			}
		}
	
		// Main requirements
		if (!requirements.IsDefaultGroupEmpty())
		{
			KxXMLNode mainReqsNode = configRootNode.NewElement("moduleDependencies");
			WriteRequirements(mainReqsNode, requirements.GetDefaultGroup());
	
			// This looks like ideal place to write format version.
			// Unfortunately, FOMM versions (that is, versions of Fallout Mod Manager) is different from my own,
			// and there is no way to always reliable distinguish them.
			// Besides, there is not much point in writing format version into FOMod.
	
			//tMainReqsNode.NewElement("fommDependency").SetAttribute("version", m_ProjectSave->GetFormatVersion());
		}
	
		// Write required files
		if (!components.GetRequiredFileData().empty())
		{
			WriteFileData(configRootNode.NewElement("requiredInstallFiles"), components.GetRequiredFileData());
		}
	
		// Write manual steps
		if (!components.GetSteps().empty())
		{
			KxXMLNode stepsArrayNode = configRootNode.NewElement("installSteps");
			stepsArrayNode.SetAttribute("order", "Explicit");
	
			for (const auto& step: components.GetSteps())
			{
				KxXMLNode stepNode = stepsArrayNode.NewElement("installStep");
				stepNode.SetAttribute("name", step->GetName());
	
				// Write step conditions
				const ConditionGroup& conditions = step->GetConditionGroup();
				if (conditions.HasConditions())
				{
					KxXMLNode stepConditionsNode = stepNode.NewElement("visible").NewElement("dependencies");
					WriteConditionGroup(conditions, stepConditionsNode);
				}
	
				KxXMLNode optionalFileGroups = stepNode.NewElement("optionalFileGroups");
				optionalFileGroups.SetAttribute("order", "Explicit");
	
				if (!step->GetGroups().empty())
				{
					for (const auto& group: step->GetGroups())
					{
						KxXMLNode setNode = optionalFileGroups.NewElement("group");
						setNode.SetAttribute("name", group->GetName());
						setNode.SetAttribute("type", ConvertSelectionMode(group->GetSelectionMode()));
	
						KxXMLNode pluginsNode = setNode.NewElement("plugins");
						pluginsNode.SetAttribute("order", "Explicit");
	
						for (const auto& entry: group->GetItems())
						{
							KxXMLNode entryNode = pluginsNode.NewElement("plugin");
							entryNode.SetAttribute("name", entry->GetName());
							
							// Description. XML scheme requires the node to be always present even if the description is empty. 
							entryNode.NewElement("description").SetValue(entry->GetDescription());
							
							// Image
							if (!entry->GetImage().IsEmpty())
							{
								entryNode.NewElement("image").SetAttribute("path", PathNameToPackage(entry->GetImage(), ContentType::Images));
							}

							// FOMod always requires 'files' node, so no check for empty array
							WriteFileData(entryNode.NewElement("files"), entry->GetFileData());
	
							// Assigned flags
							if (entry->GetConditionFlags().HasFlags())
							{
								WriteAssignedFlags(entry->GetConditionFlags(), entryNode.NewElement("conditionFlags"));
							}
	
							// Type descriptor
							// In FOMod this thing implements flags and requirements checking
							KxXMLNode typeDescriptorNode = entryNode.NewElement("typeDescriptor");
							if (!entry->GetTDConditionGroup().HasConditions() && entry->GetRequirements().empty())
							{
								// Simple variant - no requirements and no conditions.
								wxString typeDescriptor = components.TypeDescriptorToString(entry->GetTDDefaultValue());
								typeDescriptorNode.NewElement("type").SetAttribute("name", typeDescriptor);
							}
							else
							{
								// Extended variant
								KxXMLNode dependencyTypeNode = typeDescriptorNode.NewElement("dependencyType");
								dependencyTypeNode.NewElement("defaultType").SetAttribute("name", components.TypeDescriptorToString(entry->GetTDDefaultValue()));
	
								KxXMLNode patternNode = dependencyTypeNode.NewElement("patterns").NewElement("pattern");
								KxXMLNode dependenciesNode = patternNode.NewElement("dependencies");
								WriteConditionGroup(entry->GetTDConditionGroup(), dependenciesNode);
								WriteRequirements(dependenciesNode, entry->GetRequirements());
	
								// New type descriptor
								TypeDescriptor conditionalTD = entry->GetTDConditionalValue() != TypeDescriptor::Invalid ? entry->GetTDConditionalValue() : entry->GetTDDefaultValue();
								patternNode.NewElement("type").SetAttribute("name", components.TypeDescriptorToString(conditionalTD));
							}
						}
					}
				}
			}
		}
	
		// Conditional steps
		if (!components.GetConditionalSteps().empty())
		{
			WriteConditionalSteps(configRootNode.NewElement("conditionalFileInstalls").NewElement("patterns"));
		}
	
		// Make simple installation if no components are present
		if (components.GetSteps().empty() && components.GetConditionalSteps().empty())
		{
			wxString name = m_ProjectSave->GetModName();
			
			KxXMLNode stepsArrayNode = configRootNode.NewElement("installSteps");
			stepsArrayNode.SetAttribute("order", "Explicit");
			
			KxXMLNode stepNode = stepsArrayNode.NewElement("installStep");
			stepNode.SetAttribute("name", name);
	
			KxXMLNode optionalFileGroups = stepNode.NewElement("optionalFileGroups");
			optionalFileGroups.SetAttribute("order", "Explicit");
	
			KxXMLNode setNode = optionalFileGroups.NewElement("group");
			setNode.SetAttribute("name", name);
			setNode.SetAttribute("type", ConvertSelectionMode(SelectionMode::All));
	
			KxXMLNode pluginsNode = setNode.NewElement("plugins");
			pluginsNode.SetAttribute("order", "Explicit");
	
			// Entry
			KxXMLNode entryNode = pluginsNode.NewElement("plugin");
			entryNode.SetAttribute("name", name);
	
			if (!m_ProjectSave->GetInfo().GetDescription().IsEmpty())
			{
				entryNode.NewElement("description").SetValue(m_ProjectSave->GetInfo().GetDescription());
			}
	
			if (const ImageItem* imageItem = m_ProjectSave->GetInterface().GetMainItem())
			{
				entryNode.NewElement("image").SetAttribute("path", PathNameToPackage(imageItem->GetPath(), ContentType::Images));
			}
	
			KxXMLNode typeDescriptorNode = entryNode.NewElement("typeDescriptor");
			typeDescriptorNode.NewElement("type").SetAttribute("name", components.TypeDescriptorToString(TypeDescriptor::Required));
	
			KxStringVector fileNames;
			for (const auto& fileEntry: m_ProjectSave->GetFileData().GetItems())
			{
				fileNames.push_back(fileEntry->GetID());
			}
			WriteFileData(entryNode.NewElement("files"), fileNames, true);
		}
	}
	void FOModSerializer::WriteConditionalSteps(KxXMLNode& stepsArrayNode)
	{
		const ComponentsSection& components = m_ProjectSave->GetComponents();
		for (const auto& step: components.GetConditionalSteps())
		{
			KxXMLNode stepNode = stepsArrayNode.NewElement("pattern");
	
			// Dependencies
			WriteConditionGroup(step->GetConditionGroup(), stepNode.NewElement("dependencies"));
	
			// Files
			WriteFileData(stepNode.NewElement("files"), step->GetItems());
		}
	}
	void FOModSerializer::WriteFileData(KxXMLNode& node, const KxStringVector& files, bool alwaysInstall)
	{
		const FileDataSection& fileData = m_ProjectSave->GetFileData();
		for (const wxString& id: files)
		{
			if (const FileItem* file = fileData.FindItemWithID(id))
			{
				KxXMLNode fileNode = node.NewElement(file->QueryInterface<PackageProject::FolderItem>() ? "folder" : "file");
	
				// Source
				fileNode.SetAttribute("source", file->GetID());
	
				// Destination
				if (IsRootPathHandlingNeeded())
				{
					wxString destination = file->GetDestination();
					wxString destinationL = KxString::ToLower(destination);
					wxString rootPathL = KxString::ToLower(GetDataFolderName(false));
					if (destinationL.StartsWith(rootPathL))
					{
						destination.Remove(0, rootPathL.Length());
						if (!destination.IsEmpty() && destination[0] == '\\')
						{
							destination.Remove(0, 1);
						}
					}
					fileNode.SetAttribute("destination", destination);
				}
				else
				{
					fileNode.SetAttribute("destination", file->GetDestination());
				}
	
				// Priority. XML scheme requires this attribute to be always present. Default value is 0.
				fileNode.SetAttribute("priority", !file->IsDefaultPriority() ? file->GetPriority() : 0);
	
				// Always install
				if (alwaysInstall)
				{
					fileNode.SetAttribute("alwaysInstall", "true");
				}
	
				// Install if usable
				const bool installIfUsable = false;
				if (installIfUsable)
				{
					fileNode.SetAttribute("installIfUsable", "true");
				}
			}
		}
	}
	void FOModSerializer::WriteRequirements(KxXMLNode& node, const KxStringVector& requiremetSets)
	{
		const RequirementsSection& requirements = m_ProjectSave->GetRequirements();
		const PackageDesigner::IWithScriptExtender* se = IPackageManager::GetInstance()->TryGetComponent<PackageDesigner::IWithScriptExtender>();
	
		for (const wxString& id: requiremetSets)
		{
			if (RequirementGroup* group = requirements.FindGroupWithID(id))
			{
				node.SetAttribute("operator", group->GetOperator() == Operator::And ? "And" : "Or");
				for (const auto& item: group->GetItems())
				{
					if (item->GetID() == IGameInstance::GetActive()->GetGameID())
					{
						node.NewElement("gameDependency").SetAttribute("version", item->GetRequiredVersion());
					}
					else if (se && item->GetID() == se->GetEntry().GetID())
					{
						node.NewElement("foseDependency").SetAttribute("version", item->GetRequiredVersion());
					}
					else
					{
						ObjectFunction objectFunc = item->GetObjectFunction();
						if (objectFunc == ObjectFunction::PluginActive || objectFunc == ObjectFunction::PluginInactive)
						{
							KxXMLNode dependencyNode = node.NewElement("fileDependency");
							dependencyNode.SetAttribute("file", item->GetObject());
							dependencyNode.SetAttribute("state", objectFunc == ObjectFunction::PluginActive ? "Active" : "Inactive");
						}
					}
				}
			}
		}
	}
	
	void FOModSerializer::InitDataFolderInfo()
	{
		const GameID id = IGameInstance::GetActive()->GetGameID();
	
		m_IsMorrowind = id == GameIDs::Morrowind;
		m_HasDataFolderAsRoot =
			id == GameIDs::Skyrim ||
			id == GameIDs::SkyrimSE ||
			id == GameIDs::SkyrimVR ||
			id == GameIDs::Oblivion ||
	
			id == GameIDs::Fallout3 ||
			id == GameIDs::FalloutNV ||
			id == GameIDs::Fallout4 ||
			id == GameIDs::Fallout4VR;
	}
	void FOModSerializer::Init()
	{
		InitDataFolderInfo();
	}
	
	FOModSerializer::FOModSerializer(const wxString& projectFolder)
		:m_ProjectFolder(projectFolder)
	{
		Init();
	}
	FOModSerializer::FOModSerializer(const wxString& sInfoXML, const wxString& moduleConfigXML, const wxString& projectFolder)
		:m_InfoXML(sInfoXML), m_ModuleConfigXML(moduleConfigXML), m_ProjectFolder(projectFolder)
	{
		Init();
	
		if (!m_ProjectFolder.IsEmpty() && m_ProjectFolder.Last() == '\\')
		{
			m_ProjectFolder.RemoveLast(1);
		}
	}
	
	void FOModSerializer::Serialize(const ModPackageProject& project)
	{
		m_ProjectSave = &project;
		m_XML.Load(wxEmptyString);
	
		// Info.xml
		WriteInfo();
		m_InfoXML = m_XML.GetXML();
	
		// ModuleConfig.xml
		m_XML.Load(wxEmptyString);
		WriteInstallSteps();
		m_ModuleConfigXML = m_XML.GetXML();
	}
	void FOModSerializer::Structurize(ModPackageProject& project)
	{
		m_ProjectLoad = &project;
		if (m_XML.Load(m_InfoXML))
		{
			ReadInfo();
		}
	
		if (m_XML.Load(m_ModuleConfigXML))
		{
			ReadInstallSteps();
			UniqueImages();
			UniqueStringArray(m_ProjectLoad->GetComponents().GetRequiredFileData());
		}
	}
}
