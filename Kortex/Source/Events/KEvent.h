#pragma once
#include "stdafx.h"
#include "KRTTI.h"

class KEvent: public wxNotifyEvent, public KRTTI::CastAsIs<KEvent>
{
	public:
		template <class EventT, class Function, class HandlerT>
		static void Bind(const wxEventTypeTag<EventT>& eventType, const Function& function, HandlerT handler)
		{
			wxTheApp->Bind(eventType, [function, handler](EventT& event)
			{
				(handler->*function)(event);
				event.Skip();
			});
		}
		
		template <class EventT, class FunctorT>
		static void Bind(const wxEventTypeTag<EventT>& eventType, const FunctorT& functor)
		{
			wxTheApp->Bind(eventType, [&functor](EventT& event)
			{
				functor(event);
				event.Skip();
			});
		}

	protected:
		static bool SendEvent(wxEvent& event)
		{
			return wxTheApp->ProcessEvent(event);
		}
		static void QueueEvent(wxEvent* event)
		{
			wxTheApp->QueueEvent(event);
		}

	public:
		KEvent(wxEventType type = wxEVT_NULL);
		virtual ~KEvent();
		KEvent* Clone() const override;

	public:
		bool Send()
		{
			return SendEvent(*this);
		}
		void Queue()
		{
			QueueEvent(this);
		}
};
