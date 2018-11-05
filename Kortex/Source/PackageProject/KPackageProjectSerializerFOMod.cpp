#include "stdafx.h"
#include "KPackageProjectSerializerFOMod.h"
#include "KPackageProjectSerializer.h"
#include "KPackageProject.h"
#include "KPackageProjectConfig.h"
#include "KPackageProjectInfo.h"
#include "KPackageProjectInterface.h"
#include "KPackageProjectFileData.h"
#include "KPackageProjectRequirements.h"
#include "KPackageProjectComponents.h"
#include "GameInstance/KInstanceManagement.h"
#include "PackageManager/KPackageManager.h"
#include "ModManager/KModManager.h"
#include "KAux.h"
#include "KUnsortedUnique.h"
#include <KxFramework/KxString.h>

static wxSize WriteKPPCFlagEntryArray(const KPPCFlagEntryArray& array, KxXMLNode& arrayNode, bool isRequired)
{
	int andCount = 0;
	int orCount = 0;

	arrayNode.ClearChildren();
	for (const KPPCFlagEntry& value: array)
	{
		value.GetOperator() == KPP_OPERATOR_AND ? andCount++ : orCount++;

		if (isRequired)
		{
			KxXMLNode entryNode = arrayNode.NewElement("flagDependency");
			entryNode.SetAttribute("flag", value.GetName());
			entryNode.SetAttribute("value", value.GetValue());
		}
		else
		{
			KxXMLNode entryNode = arrayNode.NewElement("flag");
			entryNode.SetValue(value.GetValue());
			entryNode.SetAttribute("name", value.GetName());
		}
	}
	return wxSize(andCount, orCount);
}
static void ReadKPPCFlagEntryArray(KPPCFlagEntryArray& array, const KxXMLNode& arrayNode, KPPOperator operatorType = KPP_OPERATOR_INVALID)
{
	operatorType != KPP_OPERATOR_INVALID ? operatorType : KPackageProjectComponents::ms_DefaultFlagsOperator;

	for (KxXMLNode node = arrayNode.GetFirstChildElement("flag"); node.IsOK(); node = node.GetNextSiblingElement("flag"))
	{
		array.emplace_back(KPPCFlagEntry(node.GetValue(), node.GetAttribute("name"), operatorType));
	}
}
template<class T> static void SortEntries(T& array, const wxString& order)
{
	if (order != "Explicit")
	{
		bool isLess = order == "Ascending";
		std::sort(array.begin(), array.end(), [isLess](const auto& v1, const auto& v2)
		{
			return isLess ? v1->GetName() < v2->GetName() : v1->GetName() > v2->GetName();
		});
	}
}

wxString KPackageProjectSerializerFOMod::GetDataFolderName(bool withSeparator) const
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
wxString KPackageProjectSerializerFOMod::MakeProjectPath(const wxString& path) const
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
KPPCSelectionMode KPackageProjectSerializerFOMod::ConvertSelectionMode(const wxString& mode) const
{	
	// Remove 'Select' from string and use internal conversion function
	return KPackageProjectComponents::StringToSelectionMode(wxString(mode).Remove(0, 6));
}
wxString KPackageProjectSerializerFOMod::ConvertSelectionMode(KPPCSelectionMode mode) const
{
	// Append internal conversion function result to 'Select'
	return "Select" + KPackageProjectComponents::SelectionModeToString(mode);
}
KPPOperator KPackageProjectSerializerFOMod::DecideOperator(const wxSize& v1, const wxSize& v2) const
{
	wxSize totalOpCount = v1 + v2;
	return totalOpCount.GetX() > totalOpCount.GetY() ? KPP_OPERATOR_AND : KPP_OPERATOR_OR;
}
KxStringVector KPackageProjectSerializerFOMod::ConvertTagsArray(const KxStringVector& FOModTags) const
{
	KxStringVector outList;
	const KModTag::Vector& allTags = KModTagsManager::GetInstance()->GetTags();
	for (const wxString& tagName: FOModTags)
	{
		if (CheckTag(tagName))
		{
			outList.push_back(tagName);
		}
	}
	return outList;
}

/* Structurize */
void KPackageProjectSerializerFOMod::ReadInfo()
{
	KxXMLNode infoNode = m_XML.QueryElement("fomod");
	if (infoNode.IsOK())
	{
		KPackageProjectInfo& info = m_ProjectLoad->GetInfo();

		// Basic info
		info.SetName(infoNode.GetFirstChildElement("Name").GetValue());
		info.SetVersion(infoNode.GetFirstChildElement("Version").GetValue());
		info.SetAuthor(infoNode.GetFirstChildElement("Author").GetValue());
		info.SetDescription(ConvertBBCode(KxString::Trim(infoNode.GetFirstChildElement("Description").GetValue(), true, true)));

		// Web-site
		int64_t intID = -1;
		wxString id = infoNode.GetFirstChildElement("Id").GetValue();
		if (!id.IsEmpty() && id.ToLongLong(&intID))
		{
			info.SetWebSite(KNETWORK_PROVIDER_ID_NEXUS, intID);
		}

		wxString siteURL = infoNode.GetFirstChildElement("Website").GetValue();
		if (!siteURL.IsEmpty())
		{
			auto AddGenericSite = [&info, &siteURL](const wxString& siteName)
			{
				info.GetWebSites().emplace_back(KLabeledValue(siteURL, siteName.AfterLast('.')));
			};

			wxString siteName;
			auto webSite = TryParseWebSite(siteURL, &siteName);
			if (webSite.first != -1 && webSite.second != KNETWORK_PROVIDER_ID_INVALID)
			{
				// Site for Nexus already retrieved
				if (webSite.second == KNETWORK_PROVIDER_ID_NEXUS && intID != -1)
				{
					AddGenericSite(siteName);
				}
				else
				{
					info.SetWebSite(webSite.second, webSite.first);
				}
			}
			else
			{
				AddGenericSite(siteName);
			}
		}

		// Load and convert tags
		KAux::LoadStringArray(info.GetTags(), infoNode.GetFirstChildElement("Groups"));
		info.GetTags() = ConvertTagsArray(info.GetTags());
	}
}

void KPackageProjectSerializerFOMod::ReadInstallSteps()
{
	KPackageProjectInfo& info = m_ProjectLoad->GetInfo();
	KPackageProjectInterface& interfaceConfig = m_ProjectLoad->GetInterface();
	KPackageProjectRequirements& requirements = m_ProjectLoad->GetRequirements();
	KPackageProjectComponents& components = m_ProjectLoad->GetComponents();

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
			KPPITitleConfig& titleConfig = interfaceConfig.GetTitleConfig();

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
		for (const auto& v: ReadFileData(configRootNode.GetFirstChildElement("requiredInstallFiles")))
		{
			components.GetRequiredFileData().emplace_back(v.first->GetID());
		}

		// Header image
		KxXMLNode headerImageNode = configRootNode.GetFirstChildElement("moduleImage");
		if (headerImageNode.IsOK())
		{
			KPPIImageEntry& entry = interfaceConfig.GetImages().emplace_back();
			entry.SetPath(MakeProjectPath(headerImageNode.GetAttribute("path")));
			entry.SetVisible(headerImageNode.GetAttributeBool("showImage", true));
			entry.SetFadeEnabled(headerImageNode.GetAttributeBool("showFade", entry.IsVisible()));
			entry.SetSize(wxSize(wxDefaultCoord, headerImageNode.GetAttributeInt("height", wxDefaultCoord)));

			interfaceConfig.SetHeaderImage(entry.GetPath());
		}

		// Main requirements
		KPPRRequirementsGroup* mainReqsGroup = NULL;
		KxXMLNode moduleReqsNode = configRootNode.GetFirstChildElement("moduleDependencies").GetFirstChildElement("dependencies");
		if (!moduleReqsNode.IsOK())
		{
			moduleReqsNode = configRootNode.GetFirstChildElement("moduleDependencies");
		}
		ReadCompositeDependencies(moduleReqsNode, NULL, NULL, &mainReqsGroup, true, "Main");
		if (mainReqsGroup)
		{
			// If main requirements group is empty - add current game with no required version
			if (mainReqsGroup->GetEntries().empty())
			{
				KPPRRequirementEntry* entry = mainReqsGroup->GetEntries().emplace_back(new KPPRRequirementEntry()).get();
				entry->SetID(KApp::Get().GetCurrentGameID());
				entry->ConformToTypeDescriptor();
			}
			requirements.GetDefaultGroup().push_back(mainReqsGroup->GetID());
		}

		/* Install steps */
		auto& steps = components.GetSteps();

		KxXMLNode tInstallStepsArrayNode = configRootNode.GetFirstChildElement("installSteps");
		wxString stepsOrder = tInstallStepsArrayNode.GetAttribute("order");
		for (KxXMLNode stepNode = tInstallStepsArrayNode.GetFirstChildElement("installStep"); stepNode.IsOK(); stepNode = stepNode.GetNextSiblingElement("installStep"))
		{
			KPPCStep* step = steps.emplace_back(new KPPCStep()).get();
			step->SetName(stepNode.GetAttribute("name"));

			// Step conditions
			{
				KxXMLNode stepConditionsNode = stepNode.GetFirstChildElement("visible").GetFirstChildElement("dependencies");
				if (!stepConditionsNode.IsOK())
				{
					stepConditionsNode = stepNode.GetFirstChildElement("visible");
				}
				ReadCompositeDependencies(stepConditionsNode, &step->GetConditions(), NULL, NULL, false, step->GetName());
			}

			KxXMLNode optionalFileGroupsNode = stepNode.GetFirstChildElement("optionalFileGroups");
			if (optionalFileGroupsNode.IsOK())
			{
				for (KxXMLNode groupNode = optionalFileGroupsNode.GetFirstChildElement("group"); groupNode.IsOK(); groupNode = groupNode.GetNextSiblingElement("group"))
				{
					KPPCGroup* group = step->GetGroups().emplace_back(new KPPCGroup()).get();
					group->SetName(groupNode.GetAttribute("name"));
					group->SetSelectionMode(ConvertSelectionMode(groupNode.GetAttribute("type")));

					KxXMLNode pluginsArrayNode = groupNode.GetFirstChildElement("plugins");
					wxString pluginsOrder = pluginsArrayNode.GetAttribute("order");

					for (KxXMLNode pluginNode = pluginsArrayNode.GetFirstChildElement("plugin"); pluginNode.IsOK(); pluginNode = pluginNode.GetNextSiblingElement("plugin"))
					{
						KPPCEntry* entry = group->GetEntries().emplace_back(new KPPCEntry()).get();
						entry->SetName(pluginNode.GetAttribute("name"));

						// Description
						wxString description = ConvertBBCode(KxString::Trim(pluginNode.GetFirstChildElement("description").GetValue(), true, true));
						entry->SetDescription(description);

						// Image
						wxString pluginImage = MakeProjectPath(pluginNode.GetFirstChildElement("image").GetAttribute("path"));
						if (!pluginImage.IsEmpty())
						{
							interfaceConfig.GetImages().emplace_back(KPPIImageEntry(pluginImage, wxEmptyString, true));
							entry->SetImage(pluginImage);
						}

						// Type descriptor (they are identical to my own since they was ported from FOMod)
						KxXMLNode typeDescriptorNode = pluginNode.GetFirstChildElement("typeDescriptor").GetFirstChildElement();
						wxString typeDescriptorNodeName = typeDescriptorNode.GetName();
						if (typeDescriptorNodeName == "type")
						{
							// Simple variant
							wxString sTypeDescriptor = typeDescriptorNode.GetAttribute("name");
							entry->SetTDDefaultValue(KPackageProjectComponents::StringToTypeDescriptor(sTypeDescriptor));
						}
						else if (typeDescriptorNodeName == "dependencyType")
						{
							// Dependencies check variant

							// By scheme there may be multiple 'pattern' nodes inside 'patterns' but I haven't seen any FOMod that uses such configuration
							// nor there is any sense in doing this.
							KxXMLNode node = typeDescriptorNode.GetFirstChildElement("patterns").GetFirstChildElement("pattern");
							entry->SetTDDefaultValue(KPackageProjectComponents::StringToTypeDescriptor(typeDescriptorNode.GetFirstChildElement("defaultType").GetAttribute("name")));
							entry->SetTDConditionalValue(KPackageProjectComponents::StringToTypeDescriptor(node.GetFirstChildElement("type").GetAttribute("name")));

							wxString reqGroupID = wxString::Format("%s::%s::%s", step->GetName(), group->GetName(), entry->GetName());
							ReadCompositeDependencies(node.GetFirstChildElement("dependencies"), &entry->GetTDConditions(), entry, NULL, false, reqGroupID);
						}

						// Assigned flags
						ReadKPPCFlagEntryArray(entry->GetAssignedFlags(), pluginNode.GetFirstChildElement("conditionFlags"));

						// Files
						ReadFileData(pluginNode.GetFirstChildElement("files"), entry);
					}

					// Sort entries in set
					SortEntries(group->GetEntries(), pluginsOrder);
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
void KPackageProjectSerializerFOMod::ReadConditionalSteps(const KxXMLNode& stepsArrayNode)
{
	KPackageProjectRequirements& requirements = m_ProjectLoad->GetRequirements();
	KPackageProjectComponents& components = m_ProjectLoad->GetComponents();
	auto& tConditionalSteps = components.GetConditionalSteps();

	size_t index = 1;
	for (KxXMLNode stepNode = stepsArrayNode.GetFirstChildElement("pattern"); stepNode.IsOK(); stepNode = stepNode.GetNextSiblingElement("pattern"))
	{
		KPPCConditionalStep* step = tConditionalSteps.emplace_back(new KPPCConditionalStep()).get();

		// Files
		for (const auto& v: ReadFileData(stepNode.GetFirstChildElement("files")))
		{
			step->GetEntries().emplace_back(v.first->GetID());
		}

		// Conditions
		KPPRRequirementsGroup* reqSet = NULL;
		KPPOperator operatorType = ReadCompositeDependencies(stepNode.GetFirstChildElement("dependencies"), &step->GetConditions(), NULL, &reqSet);
		if (reqSet)
		{
			reqSet->SetID(wxString::Format("ConditionalStep#%zu", index));
			step->GetConditions().emplace_back(KPPCFlagEntry("true", reqSet->GetFlagName(), operatorType));
		}
	}
}
KPackageProjectSerializerFOMod::FilePriorityArray KPackageProjectSerializerFOMod::ReadFileData(const KxXMLNode& filesArrayNode, KPPCEntry* entry)
{
	FilePriorityArray priorityList;
	if (filesArrayNode.IsOK())
	{
		for (KxXMLNode fileDataNode = filesArrayNode.GetFirstChildElement(); fileDataNode.IsOK(); fileDataNode = fileDataNode.GetNextSiblingElement())
		{
			KPPFFileEntry* fileEntry = NULL;
			if (fileDataNode.GetName() == "folder")
			{
				fileEntry = new KPPFFolderEntry();
			}
			else
			{
				fileEntry = new KPPFFileEntry();
			}

			wxString source = fileDataNode.GetAttribute("source");
			fileEntry->SetID(source);
			fileEntry->SetSource(MakeProjectPath(source));
			fileEntry->SetPriority(fileDataNode.GetAttributeInt("priority", KPackageProjectFileData::ms_DefaultPriority));

			wxString destination = fileDataNode.GetAttribute("destination");
			fileEntry->SetDestination(GetDataFolderName(true) + (!destination.IsEmpty() ? destination : wxEmptyString));

			priorityList.push_back(std::make_pair(fileEntry, fileDataNode.GetAttributeInt("priority", std::numeric_limits<int64_t>::max())));

			// Decide whether to add this item to required files or not
			bool shouldAlwaysInstall = fileDataNode.GetAttributeBool("alwaysInstall", false);
			bool shouldInstallIfUsable = fileDataNode.GetAttributeBool("installIfUsable", false);
			if (shouldAlwaysInstall || shouldInstallIfUsable && entry->GetTDDefaultValue() != KPPC_DESCRIPTOR_NOT_USABLE)
			{
				m_ProjectLoad->GetComponents().GetRequiredFileData().emplace_back(fileEntry->GetID());
			}
		}

		// Sort by priority
		std::sort(priorityList.begin(), priorityList.end(), [](const auto& v1, const auto& v2)
		{
			return v1.second < v2.second;
		});

		// Link these files to entry
		if (entry)
		{
			for (const auto& v: priorityList)
			{
				entry->GetFileData().emplace_back(v.first->GetID());
			}
		}

		// Add to project
		KPackageProjectFileData& fileData = m_ProjectLoad->GetFileData();
		for (const auto& v: priorityList)
		{
			fileData.AddFile(v.first);
		}
	}
	return priorityList;
}
KPPOperator KPackageProjectSerializerFOMod::ReadCompositeDependencies(const KxXMLNode& dependenciesNode,
																	  KPPCFlagEntryArray* conditions,
																	  KPPCEntry* componentsEntry,
																	  KPPRRequirementsGroup** reqGroupOut,
																	  bool alwaysCreateReqGroup,
																	  const wxString& createdReqGroupID
)
{
	KPackageProjectRequirements& requirements = m_ProjectLoad->GetRequirements();

	// These 'compositeDependency' structs can have an operator (default AND).
	// I can apply it to flag and requirement entry as FOMod doesn't support operators for individual entries.
	KPPOperator operatorType = dependenciesNode.GetAttribute("operator") == "Or" ? KPP_OPERATOR_OR : KPP_OPERATOR_AND;

	KPPRRequirementsGroup* reqGroup = NULL;
	if (alwaysCreateReqGroup)
	{
		reqGroup = requirements.GetGroups().emplace_back(new KPPRRequirementsGroup()).get();
		reqGroup->SetID(createdReqGroupID);
	}

	for (KxXMLNode depNode = dependenciesNode.GetFirstChildElement(); depNode.IsOK(); depNode = depNode.GetNextSiblingElement())
	{
		KPPRRequirementEntry* reqEntry = NULL;

		wxString name = depNode.GetName();
		if (name == "fileDependency")
		{
			reqEntry = new KPPRRequirementEntry();
			reqEntry->SetObject(depNode.GetAttribute("file"));

			// FOMod support only these three required states
			wxString state = depNode.GetAttribute("state");
			if (state == "Active")
			{
				reqEntry->SetObjectFunction(KPPR_OBJFUNC_PLUGIN_ACTIVE);
			}
			else if (state == "Inactive")
			{
				reqEntry->SetObjectFunction(KPPR_OBJFUNC_PLUGIN_INACTIVE);
			}
			else if (state == "Missing")
			{
				reqEntry->SetObjectFunction(KPPR_OBJFUNC_FILE_NOT_EXIST);
			}
			else
			{
				reqEntry->SetObjectFunction(KPPR_OBJFUNC_NONE);
			}
		}
		else if (name == "gameDependency")
		{
			reqEntry = new KPPRRequirementEntry();

			// Copy std requirement for current game and set required version from FOMod
			const KPPRRequirementEntry* stdEntry = KPackageManager::GetInstance()->FindStdReqirement(KApp::Get().GetCurrentGameID());
			
			// This check probably redundant, but just in case
			if (stdEntry)
			{
				*reqEntry = *stdEntry;
			}
			else
			{
				reqEntry->SetObjectFunction(KPPR_OBJFUNC_FILE_EXIST);
			}
			reqEntry->SetRequiredVersion(depNode.GetAttribute("version"));
		}
		else if (name == "foseDependency")
		{
			reqEntry = new KPPRRequirementEntry();

			// Although it's named 'fose' I will interpret this as generic Script Extender requirement
			const KPPRRequirementEntry* stdEntry = KPackageManager::GetInstance()->FindScriptExtenderRequirement();

			// There may be no Script Extender
			if (stdEntry)
			{
				*reqEntry = *stdEntry;
			}
			else
			{
				// Fill with something meaningful
				reqEntry->SetName(KGameInstance::GetActive()->GetShortName() + " Script Extender");
				reqEntry->SetObjectFunction(KPPR_OBJFUNC_FILE_EXIST);
			}
		}
		else if (name == "flagDependency")
		{
			// This is an equivalent of KMP's 'Conditions' attribute.
			if (conditions)
			{
				wxString name = depNode.GetAttribute("flag");
				wxString value = depNode.GetAttribute("value");
				conditions->emplace_back(KPPCFlagEntry(value, name, operatorType));
			}
		}
		else if (name == "fommDependency")
		{
			// This could be interpreted as mod manager version.
			// KMP doesn't currently support that. It has a 'FormatVersion' attribute,
			// but versions used in FOMod will surely be different from versions
			// of Kortex's install engine. So ignore this for now.
		}
		else
		{
			// There also can be 'dependencies' element which can include another level of this structure.
			// This seems wrong and I will not process this.
		}

		// Configure and add the requirement to its set
		if (reqEntry)
		{
			reqEntry->SetOperator(operatorType);

			// If no name assigned to this entry (which is always the case right now),
			// extract name from file path.
			if (reqEntry->GetName().IsEmpty())
			{
				reqEntry->SetName(reqEntry->GetObject().AfterLast('\\').BeforeLast('.'));
			}

			// Try set type to system
			reqEntry->TrySetTypeDescriptor(KPPR_TYPE_SYSTEM);

			// Create requirements group if needed and it haven't created earlier
			if (!reqGroup)
			{
				reqGroup = requirements.GetGroups().emplace_back(new KPPRRequirementsGroup()).get();
				reqGroup->SetID(createdReqGroupID);
			}
			reqGroup->GetEntries().emplace_back(reqEntry);
		}
	}

	// Set this group as flag requirement to conditions list
	if (conditions && reqGroup)
	{
		conditions->emplace_back(KPPCFlagEntry("true", reqGroup->GetFlagName(), operatorType));
	}

	// Link this requirements group to provided component
	if (componentsEntry && reqGroup)
	{
		componentsEntry->GetRequirements().emplace_back(reqGroup->GetID());
	}

	if (reqGroupOut)
	{
		*reqGroupOut = reqGroup;
	}
	return operatorType;
}
void KPackageProjectSerializerFOMod::UniqueFileData()
{
	auto& files = m_ProjectLoad->GetFileData().GetData();
	auto it = std::unique(files.begin(), files.end(), [](const auto& v1, const auto& v2)
	{
		return v1->GetID() == v2->GetID();
	});
	files.erase(it, files.end());
}
void KPackageProjectSerializerFOMod::UniqueImages()
{
	KPPIImageEntryArray& images = m_ProjectLoad->GetInterface().GetImages();
	auto it = KUnsortedUnique(images.begin(), images.end(), [](const KPPIImageEntry& v1, const KPPIImageEntry& v2)
	{
		return v1.GetPath() == v2.GetPath();
	},
	[](const KPPIImageEntry& v1, const KPPIImageEntry& v2)
	{
		return v1.GetPath() < v2.GetPath();
	});
	images.erase(it, images.end());
}

/* Serialize */
void KPackageProjectSerializerFOMod::WriteInfo()
{
	const KPackageProjectInfo& info = m_ProjectSave->GetInfo();
	KxXMLNode infoNode = m_XML.NewElement("fomod");

	infoNode.NewElement("Name").SetValue(info.GetName());
	infoNode.NewElement("Author").SetValue(info.GetAuthor());

	KxXMLNode versionNode = infoNode.NewElement("Version");
	versionNode.SetValue(info.GetVersion());
	versionNode.SetAttribute("MachineVersion", "5.0");

	if (!info.GetDescription().IsEmpty())
	{
		infoNode.NewElement("Description").SetValue(info.GetDescription(), true);
	}

	WriteSites(infoNode, infoNode.NewElement("Website"));
	KAux::SaveStringArray(info.GetTags(), infoNode.NewElement("Groups"), "element");
}
void KPackageProjectSerializerFOMod::WriteSites(KxXMLNode& infoNode, KxXMLNode& sitesNode)
{
	// FOMod supports only one web-site and field for site ID, so I need to decide which one to write.
	// The order will be: Nexus -> Id, TESALL -> LoversLab -> other (if any)

	const KPackageProjectInfo& info = m_ProjectSave->GetInfo();
	auto WriteSite = [&info, &sitesNode](KNetworkProviderID index) -> bool
	{
		if (info.HasWebSite(index))
		{
			sitesNode.SetValue(info.GetWebSite(index).GetValue());
			return true;
		}
		return false;
	};

	// Write Nexus to 'Id'
	if (info.HasWebSite(KNETWORK_PROVIDER_ID_NEXUS))
	{
		infoNode.NewElement("Id").SetValue(info.GetWebSiteModID(KNETWORK_PROVIDER_ID_NEXUS));
	}

	if (!(WriteSite(KNETWORK_PROVIDER_ID_TESALL) || WriteSite(KNETWORK_PROVIDER_ID_LOVERSLAB)))
	{
		// Write first one from list
		const KLabeledValueArray& tSites = info.GetWebSites();
		if (!tSites.empty())
		{
			sitesNode.SetValue(tSites[0].GetValue());
		}
	}

	// Ignore all others sites
}

void KPackageProjectSerializerFOMod::WriteInstallSteps()
{
	const KPackageProjectInterface& interfaceConfig = m_ProjectSave->GetInterface();
	const KPackageProjectRequirements& requirements = m_ProjectSave->GetRequirements();
	const KPackageProjectComponents& components = m_ProjectSave->GetComponents();

	KxXMLNode configRootNode = m_XML.NewElement("config");

	// Write XML-Schema
	if (KPackageManager::GetInstance()->GetOptions().GetAttributeBool("FOModUseHTTPSForXMLScheme", true))
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
	moduleNameNode.SetValue(m_ProjectSave->ComputeModID());

	// Write title customization
	const KPPITitleConfig& titleConfig = interfaceConfig.GetTitleConfig();
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
		wxString sColorValue = titleConfig.GetColor().GetAsString(KxC2S_HTML_SYNTAX).AfterFirst('#');
		if (!sColorValue.IsEmpty())
		{
			moduleNameNode.SetAttribute("colour", sColorValue);
		}
	}

	// Write header image
	if (const KPPIImageEntry* headerImageEntry = interfaceConfig.GetHeaderImageEntry())
	{
		KxXMLNode node = configRootNode.NewElement("moduleImage");
		node.SetAttribute("path", PathNameToPackage(headerImageEntry->GetPath(), KPP_CONTENT_IMAGES));
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
			if (!step->GetConditions().empty())
			{
				KxXMLNode tStepConditionsNode = stepNode.NewElement("visible").NewElement("dependencies");
				wxSize tFlagsCount = WriteKPPCFlagEntryArray(step->GetConditions(), tStepConditionsNode, true);

				KPPOperator operatorType = DecideOperator(tFlagsCount);
				tStepConditionsNode.SetAttribute("operator", operatorType == KPP_OPERATOR_AND ? "And" : "Or");
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

					for (const auto& entry: group->GetEntries())
					{
						KxXMLNode entryNode = pluginsNode.NewElement("plugin");
						entryNode.SetAttribute("name", entry->GetName());

						// Description
						if (!entry->GetDescription().IsEmpty())
						{
							entryNode.NewElement("description").SetValue(entry->GetDescription(), true);
						}

						// Image
						if (!entry->GetImage().IsEmpty())
						{
							entryNode.NewElement("image").SetAttribute("path", PathNameToPackage(entry->GetImage(), KPP_CONTENT_IMAGES));
						}

						// FOMod always requires 'files' node, so no check for empty array
						WriteFileData(entryNode.NewElement("files"), entry->GetFileData());

						// Assigned flags
						if (!entry->GetAssignedFlags().empty())
						{
							WriteKPPCFlagEntryArray(entry->GetAssignedFlags(), entryNode.NewElement("conditionFlags"), false);
						}

						// Type descriptor
						// In FOMod this thing implements flags and requirements checking
						KxXMLNode typeDescriptorNode = entryNode.NewElement("typeDescriptor");
						if (entry->GetTDConditions().empty() && entry->GetRequirements().empty())
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
							wxSize opCountFlags = WriteKPPCFlagEntryArray(entry->GetTDConditions(), dependenciesNode, true);
							wxSize opCountReqs = WriteRequirements(dependenciesNode, entry->GetRequirements());

							// Since FOMod doesn't support operators for individual entries I need to count the most used one.
							KPPOperator operatorType = DecideOperator(opCountFlags, opCountReqs);
							dependenciesNode.SetAttribute("operator", operatorType == KPP_OPERATOR_AND ? "And" : "Or");

							// New type descriptor
							KPPCTypeDescriptor conditionalTD = entry->GetTDConditionalValue() != KPPC_DESCRIPTOR_INVALID ? entry->GetTDConditionalValue() : entry->GetTDDefaultValue();
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

	// Make simple installation if no components present
	if (components.GetSteps().empty() && components.GetConditionalSteps().empty())
	{
		wxString name = m_ProjectSave->ComputeModName();

		KxXMLNode stepsArrayNode = configRootNode.NewElement("installSteps");
		stepsArrayNode.SetAttribute("order", "Explicit");

		KxXMLNode stepNode = stepsArrayNode.NewElement("installStep");
		stepNode.SetAttribute("name", name);

		KxXMLNode optionalFileGroups = stepNode.NewElement("optionalFileGroups");
		optionalFileGroups.SetAttribute("order", "Explicit");

		KxXMLNode setNode = optionalFileGroups.NewElement("group");
		setNode.SetAttribute("name", name);
		setNode.SetAttribute("type", ConvertSelectionMode(KPPC_SELECT_ALL));

		KxXMLNode pluginsNode = setNode.NewElement("plugins");
		pluginsNode.SetAttribute("order", "Explicit");

		// Entry
		KxXMLNode entryNode = pluginsNode.NewElement("plugin");
		entryNode.SetAttribute("name", name);

		if (!m_ProjectSave->GetInfo().GetDescription().IsEmpty())
		{
			entryNode.NewElement("description").SetValue(m_ProjectSave->GetInfo().GetDescription(), true);
		}

		if (const KPPIImageEntry* pImage = m_ProjectSave->GetInterface().GetMainImageEntry())
		{
			entryNode.NewElement("image").SetAttribute("path", PathNameToPackage(pImage->GetPath(), KPP_CONTENT_IMAGES));
		}

		KxXMLNode typeDescriptorNode = entryNode.NewElement("typeDescriptor");
		typeDescriptorNode.NewElement("type").SetAttribute("name", components.TypeDescriptorToString(KPPC_DESCRIPTOR_REQUIRED));

		KxStringVector fileNames;
		for (const auto& fileEntry: m_ProjectSave->GetFileData().GetData())
		{
			fileNames.push_back(fileEntry->GetID());
		}
		WriteFileData(entryNode.NewElement("files"), fileNames, true);
	}
}
void KPackageProjectSerializerFOMod::WriteConditionalSteps(KxXMLNode& stepsArrayNode)
{
	const KPackageProjectComponents& components = m_ProjectSave->GetComponents();
	for (const auto& step: components.GetConditionalSteps())
	{
		KxXMLNode stepNode = stepsArrayNode.NewElement("pattern");

		// Dependencies
		KxXMLNode tDependencies = stepNode.NewElement("dependencies");
		wxSize tOpCountFlags = WriteKPPCFlagEntryArray(step->GetConditions(), tDependencies, true);

		KPPOperator operatorType = DecideOperator(tOpCountFlags);
		tDependencies.SetAttribute("operator", operatorType == KPP_OPERATOR_AND ? "And" : "Or");

		// Files
		WriteFileData(stepNode.NewElement("files"), step->GetEntries());
	}
}
void KPackageProjectSerializerFOMod::WriteFileData(KxXMLNode& node, const KxStringVector& files, bool alwaysInstall)
{
	const KPackageProjectFileData& fileData = m_ProjectSave->GetFileData();
	for (const wxString& id: files)
	{
		const KPPFFileEntry* file = fileData.FindEntryWithID(id);
		if (file)
		{
			KxXMLNode fileNode = node.NewElement(file->ToFolderEntry() ? "folder" : "file");

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

			// Priority
			if (!file->IsDefaultPriority())
			{
				fileNode.SetAttribute("priority", file->GetPriority());
			}

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
wxSize KPackageProjectSerializerFOMod::WriteRequirements(KxXMLNode& node, const KxStringVector& requiremetSets)
{
	int andCount = 0;
	int orCount = 0;

	const KPackageProjectRequirements& requirements = m_ProjectSave->GetRequirements();
	for (const wxString& id: requiremetSets)
	{
		KPPRRequirementsGroup* group = requirements.FindGroupWithID(id);
		if (group)
		{
			const KPPRRequirementEntry* scriptExtenderReqEntry = KPackageManager::GetInstance()->FindScriptExtenderRequirement();
			for (const auto& entry: group->GetEntries())
			{
				entry->GetOperator() == KPP_OPERATOR_AND ? andCount++ : orCount++;

				if (entry->GetID() == KApp::Get().GetCurrentGameID())
				{
					node.NewElement("gameDependency").SetAttribute("version", entry->GetRequiredVersion());
				}
				else if (scriptExtenderReqEntry && entry->GetID() == scriptExtenderReqEntry->GetID())
				{
					node.NewElement("foseDependency").SetAttribute("version", entry->GetRequiredVersion());
				}
				else
				{
					KPPRObjectFunction objectFunc = entry->GetObjectFunction();
					if (objectFunc == KPPR_OBJFUNC_PLUGIN_ACTIVE || objectFunc == KPPR_OBJFUNC_PLUGIN_INACTIVE)
					{
						KxXMLNode tDepNode = node.NewElement("fileDependency");
						tDepNode.SetAttribute("file", entry->GetObject());
						tDepNode.SetAttribute("state", objectFunc == KPPR_OBJFUNC_PLUGIN_ACTIVE ? "Active" : "Inactive");
					}
				}
			}
		}
	}

	return wxSize(andCount, orCount);
}

void KPackageProjectSerializerFOMod::InitDataFolderInfo()
{
	wxString id = KApp::Get().GetCurrentGameID();
	m_HasDataFolderAsRoot = id == "Skyrim" || id == "SkyrimSE" || id == "Oblivion" || id == "Fallout3" || id == "FalloutNV" || id == "Fallout4";
	m_IsMorrowind = id == "Morrowind";
}
void KPackageProjectSerializerFOMod::Init()
{
	InitDataFolderInfo();
}

KPackageProjectSerializerFOMod::KPackageProjectSerializerFOMod(const wxString& projectFolder)
	:m_ProjectFolder(projectFolder)
{
	Init();
}
KPackageProjectSerializerFOMod::KPackageProjectSerializerFOMod(const wxString& sInfoXML, const wxString& moduleConfigXML, const wxString& projectFolder)
	:m_InfoXML(sInfoXML), m_ModuleConfigXML(moduleConfigXML), m_ProjectFolder(projectFolder)
{
	Init();

	if (!m_ProjectFolder.IsEmpty() && m_ProjectFolder.Last() == '\\')
	{
		m_ProjectFolder.RemoveLast(1);
	}
}

void KPackageProjectSerializerFOMod::Serialize(const KPackageProject* project)
{
	m_ProjectSave = project;
	m_XML.Load(wxEmptyString);

	// Info.xml
	WriteInfo();
	m_InfoXML = m_XML.GetXML();

	// ModuleConfig.xml
	m_XML.Load(wxEmptyString);
	WriteInstallSteps();
	m_ModuleConfigXML = m_XML.GetXML();
}
void KPackageProjectSerializerFOMod::Structurize(KPackageProject* project)
{
	m_ProjectLoad = project;
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
