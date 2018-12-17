#pragma once
#include "stdafx.h"
#include <KxFramework/KxRTTI.h>
#include <typeinfo>

namespace Kortex::RTTI
{
	using IID = std::size_t;

	template<class T> IID GetIIDOf() noexcept
	{
		return typeid(T).hash_code();
	}
}

namespace Kortex::RTTI
{
	class IObject
	{
		public:
			using IID = RTTI::IID;

		protected:
			virtual bool OnQueryInterface(IObject*& object, const IID& iid) = 0;

		public:
			virtual ~IObject() = default;

		public:
			IObject* QueryInterface(const IID& uid)
			{
				IObject* object = nullptr;
				OnQueryInterface(object, uid);
				return object;
			}
			const IObject* QueryInterface(const IID& uid) const
			{
				IObject* object = nullptr;
				const_cast<IObject*>(this)->OnQueryInterface(object, uid);
				return object;
			}

			template<class T> T* QueryInterface()
			{
				return static_cast<T*>(QueryInterface(GetIIDOf<T>()));
			}
			template<class T> const T* QueryInterface() const
			{
				return static_cast<const T*>(QueryInterface(GetIIDOf<T>()));
			}

			template<class T> bool QueryInterface(T*& ptr)
			{
				ptr = QueryInterface<T>();
				return ptr != nullptr;
			}
			template<class T> bool QueryInterface(const T*& ptr) const
			{
				ptr = QueryInterface<T>();
				return ptr != nullptr;
			}
	};
}

namespace Kortex::RTTI
{
	template<class I> class IInterface: public IObject
	{
		public:
			using IID = RTTI::IID;

		protected:
			virtual bool OnQueryInterface(IObject*& object, const IID& iid) override
			{
				static const IID ms_IID = GetIIDOf<I>();
				if (iid == ms_IID)
				{
					object = static_cast<I*>(this);
					return true;
				}
				
				object = nullptr;
				return false;
			}
	};
	template<class I, class... BaseInterfaces> class IMultiInterface: public BaseInterfaces...
	{
		public:
			using IID = RTTI::IID;

		protected:
			virtual bool OnQueryInterface(IObject*& object, const IID& iid) override
			{
				static const IID ms_IID = GetIIDOf<I>();
				if (iid == ms_IID)
				{
					object = static_cast<I*>(this);
					return true;
				}
				return (... || BaseInterfaces::OnQueryInterface(object, iid));
			}
	};
}

namespace Kortex::RTTI
{
	namespace Internal
	{
		template<class T> class IObjectWrapper: public IObject {};
	}

	template<class... T> class IImplementation: public Internal::IObjectWrapper<IImplementation<T...>>, public T...
	{
		public:
			using IID = RTTI::IID;

		private:
			using Wrapper = Internal::IObjectWrapper<IImplementation<T...>>;

		protected:
			virtual bool OnQueryInterface(IObject*& object, const IID& iid) override
			{
				static const IID ms_IID = GetIIDOf<IObject>();
				if (iid == ms_IID)
				{
					object = static_cast<Wrapper*>(this);
					return true;
				}
				return (T::OnQueryInterface(object, iid) || ...);
			}

		public:
			IObject* QueryInterface(const IID& uid)
			{
				IObject* object = nullptr;
				OnQueryInterface(object, uid);
				return object;
			}
			const IObject* QueryInterface(const IID& uid) const
			{
				IObject* object = nullptr;
				const_cast<IObject*>(this)->OnQueryInterface(object, uid);
				return object;
			}

			template<class T> T* QueryInterface()
			{
				if constexpr(std::is_same_v<T, IObject>)
				{
					return static_cast<Wrapper*>(this);
				}
				else
				{
					return static_cast<T*>(QueryInterface(GetIIDOf<T>()));
				}
			}
			template<class T> const T* QueryInterface() const
			{
				if constexpr(std::is_same_v<T, IObject>)
				{
					return static_cast<const Wrapper*>(this);
				}
				else
				{
					return static_cast<const T*>(QueryInterface(GetIIDOf<T>()));
				}
			}

			template<class T> bool QueryInterface(T*& ptr)
			{
				ptr = QueryInterface<T>();
				return ptr != nullptr;
			}
			template<class T> bool QueryInterface(const T*& ptr) const
			{
				ptr = QueryInterface<T>();
				return ptr != nullptr;
			}
	};
}
