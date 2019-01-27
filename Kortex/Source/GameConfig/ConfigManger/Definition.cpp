#include "stdafx.h"
#include "Definition.h"
#include "ITypeDetector.h"
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	void Definition::LoadGroups(const KxXMLNode& groupsNode)
	{
		for (KxXMLNode node = groupsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			wxString id = node.GetAttribute(wxS("ID"));
			m_Groups.insert_or_assign(id, std::make_unique<ItemGroup>(*this, id, node, m_Options));
		}
	}

	Definition::~Definition()
	{
	}

	bool Definition::Load()
	{
		KxFileStream stream(m_FilePath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		if (KxXMLDocument xml(stream); xml.IsOK())
		{
			const KxXMLNode rootNode = xml.GetFirstChildElement("Definition");

			// Load data types
			for (KxXMLNode node = rootNode.GetFirstChildElement("DataTypes").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				DataType type(node);
				if (type.IsOK())
				{
					m_DataTypes.insert_or_assign(type.GetID(), std::move(type));
				}
			}

			// Load type detectors config
			for (KxXMLNode node = rootNode.GetFirstChildElement("TypeDetection").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				switch (TypeDetectorDef::FromString(node.GetName(), TypeDetectorID::None))
				{
					case TypeDetectorID::HungarianNotation:
					{
						m_TypeDetectors.emplace_back(std::make_unique<HungarianNotationTypeDetector>(node));
						break;
					}
					case TypeDetectorID::DataAnalysis:
					{
						m_TypeDetectors.emplace_back(std::make_unique<DataAnalysisTypeDetector>());
						break;
					}
				};
			}

			// Load options
			m_Options.Load(rootNode.GetFirstChildElement(wxS("Options")));

			// Load groups
			LoadGroups(rootNode.GetFirstChildElement(wxS("Groups")));

			return true;
		}
		return false;
	}
	DataType Definition::GetDataType(TypeID id) const
	{
		auto it = m_DataTypes.find(id);
		if (it != m_DataTypes.end())
		{
			return it->second;
		}
		return DataType::CreateGeneric(id);
	}
}
