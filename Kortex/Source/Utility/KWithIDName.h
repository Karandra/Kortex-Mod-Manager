#pragma once
#include "stdafx.h"

class KWithID
{
	private:
		wxString m_ID;

	public:
		KWithID(const wxString& id = wxEmptyString)
			:m_ID(id)
		{
		}
		virtual ~KWithID()
		{
		}

	public:
		bool IsEmptyID() const
		{
			return m_ID.IsEmpty();
		}
		const wxString& RawGetID() const
		{
			return m_ID;
		}
		virtual const wxString& GetID() const
		{
			return m_ID;
		}
		void SetID(const wxString& id)
		{
			m_ID = id;
		}
};

class KWithName
{
	private:
		wxString m_Name;

	public:
		KWithName(const wxString& name = wxEmptyString)
			:m_Name(name)
		{
		}
		virtual ~KWithName()
		{
		}

	public:
		bool IsEmptyName() const
		{
			return m_Name.IsEmpty();
		}
		const wxString& RawGetName() const
		{
			return m_Name;
		}
		virtual const wxString& GetName() const
		{
			return m_Name;
		}
		void SetName(const wxString& value)
		{
			m_Name = value;
		}
};

// Tiny class which stores strings name and ID.
// Its important feature that for ID request it will return name if ID is empty and
// return for name request will return ID if name is empty.
// If this is undesirable you can query 'raw' attributes.
class KWithIDName: public KWithID, public KWithName
{
	public:
		KWithIDName(const wxString& id = wxEmptyString, const wxString& name = wxEmptyString)
			:KWithID(id), KWithName(name)
		{
		}

	public:
		virtual const wxString& GetID() const override
		{
			return IsEmptyID() ? RawGetName() : RawGetID();
		}
		virtual const wxString& GetName() const override
		{
			return IsEmptyName() ? RawGetID() : RawGetName();
		}
};
