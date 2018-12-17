#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "IPC/KIPCRequest.h"
class KIPCClient;
class KIPCServer;

class KIPCConnection: public wxConnection
{
	public:
		constexpr static bool IsServer()
		{
			return KIPC_SERVER;
		}
		constexpr static bool IsClient()
		{
			return !IsServer();
		}

	private:
		KIPCServer* m_Server = nullptr;
		KIPCClient* m_Client = nullptr;

	private:
		using SendFunctionType = bool(KIPCConnection::*)(const wxString&, const void*, size_t, wxIPCFormat);
		
		template<class T> static void AssertRequest()
		{
			static_assert(std::is_trivially_copyable_v<T>, "T is not trivially copyable");
		}
		template<class T> bool SendRequestBase(const T& request, SendFunctionType func)
		{
			AssertRequest<T>();
			wxLogInfo("Client: Sending request \"%s\"", request.GetTypeName());

			return (this->*func)(request.GetTypeName(), &static_cast<const KIPCRequest&>(request), sizeof(T), wxIPC_PRIVATE);
		}

	protected:
		virtual bool OnDisconnect() override;
		virtual bool OnPoke(const wxString& topic, const wxString& item, const void* data, size_t size, wxIPCFormat format) override;
		virtual bool OnAdvise(const wxString& topic, const wxString& item, const void* data, size_t size, wxIPCFormat format) override;
		virtual bool OnStartAdvise(const wxString& topic, const wxString& item) override;
		virtual bool OnStopAdvise(const wxString& topic, const wxString& item) override;

	public:
		KIPCConnection(KIPCServer* server = nullptr);
		KIPCConnection(KIPCClient* client = nullptr);
		virtual ~KIPCConnection();

	public:
		KIPCServer* GetServer() const
		{
			return m_Server;
		}
		KIPCClient* GetClient() const
		{
			return m_Client;
		}

		KIPCRequest::TypeID TestRequest(const wxString& item, const void* data, size_t dataSize)
		{
			wxLogInfo("Client: Receive request \"%s\"", item);
			if (data && dataSize >= sizeof(KIPCRequest))
			{
				const KIPCRequest* baseRequest = reinterpret_cast<const KIPCRequest*>(data);
				if (baseRequest->GetTypeName() == item)
				{
					return baseRequest->GetTypeID();
				}
				else
				{
					wxLogInfo("Client: invalid request type. Type \"%s\" expected, \"%s\" is received", item, baseRequest->GetTypeName());
				}
			}
			else
			{
				wxLogInfo("Client: invalid request binary data received");
			}
			return KIPCRequest::TypeID::None;
		}
		template<class T> const T& ReceiveRequest(const void* data) const
		{
			AssertRequest<T>();

			const KIPCRequest* baseRequest = reinterpret_cast<const KIPCRequest*>(data);
			return *static_cast<const T*>(baseRequest);
		}
		
		template<class T> bool SendToServer(const T& request)
		{
			return SendRequestBase(request, &KIPCConnection::Poke);
		}
		template<class T> bool SendToClient(const T& request)
		{
			return SendRequestBase(request, &KIPCConnection::Advise);
		}
};
