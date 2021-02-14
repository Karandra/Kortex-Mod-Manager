#pragma once
#include "Framework.hpp"
#include <kxf/General/IVariablesCollection.h>
#include <kxf/Serialization/XML.h>

namespace Kortex::GameInstance::Private
{
	class KORTEX_API VariableLoader final
	{
		private:
			kxf::IVariablesCollection& m_Collection;
			kxf::XMLNode m_VariablesRoot;
			kxf::String m_DefaultNamespace;

			std::function<bool(const kxf::String& type, const kxf::String& ns, const kxf::String& name, const kxf::String& value)> m_OnString;
			std::function<bool(const kxf::String& type, const kxf::String& ns, const kxf::String& name, const kxf::FSPath& value)> m_OnFSPath;
			std::function<bool(const kxf::String& type, const kxf::String& ns, const kxf::String& name, int64_t value)> m_OnInteger;

		private:
			size_t DoLoadVariables();

		public:
			VariableLoader(kxf::IVariablesCollection& collection, const kxf::XMLNode& variablesRoot, kxf::String defaultNamespace)
				:m_VariablesRoot(variablesRoot), m_Collection(collection), m_DefaultNamespace(std::move(defaultNamespace))
			{
			}

		public:
			size_t Invoke()
			{
				return DoLoadVariables();
			}

			void OnString(decltype(m_OnString) func)
			{
				m_OnString = std::move(func);
			}
			void OnFSPath(decltype(m_OnFSPath) func)
			{
				m_OnFSPath = std::move(func);
			}
			void OnInteger(decltype(m_OnInteger) func)
			{
				m_OnInteger = std::move(func);
			}
	};

	class KORTEX_API VariableSaver final
	{
		private:
			const kxf::IVariablesCollection& m_Collection;
			kxf::XMLNode m_VariablesRoot;

		private:
			size_t DoSaveVariables();

		public:
			VariableSaver(const kxf::IVariablesCollection& collection, const kxf::XMLNode& variablesRoot)
				:m_VariablesRoot(variablesRoot), m_Collection(collection)
			{
			}

		public:
			size_t Invoke()
			{
				return DoSaveVariables();
			}
	};
}
