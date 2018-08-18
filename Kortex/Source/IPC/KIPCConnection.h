#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
class KIPCRequest;
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
		KIPCServer* m_Server = NULL;
		KIPCClient* m_Client = NULL;

	private:
		typedef bool(KIPCConnection::*SendFunctionType)(const wxString&, const void*, size_t, wxIPCFormat);
		template<class T> bool SendRequestBase(const T& request, SendFunctionType func)
		{
			static_assert(std::is_trivially_copyable_v<T>, "T is not trivially copyable");

			auto itemName = request.GetClassName();
			wxLogInfo("Client: Sending request \"%s\"", itemName);

			return (this->*func)(itemName, &static_cast<const KIPCRequest&>(request), sizeof(T), wxIPC_PRIVATE);
		}

	protected:
		virtual bool OnDisconnect() override;
		virtual bool OnPoke(const wxString& topic, const wxString& item, const void* data, size_t nSize, wxIPCFormat format) override;
		virtual bool OnAdvise(const wxString& topic, const wxString& item, const void* data, size_t nSize, wxIPCFormat format) override;
		virtual bool OnStartAdvise(const wxString& topic, const wxString& item) override;
		virtual bool OnStopAdvise(const wxString& topic, const wxString& item) override;

	public:
		KIPCConnection(KIPCServer* server = NULL);
		KIPCConnection(KIPCClient* client = NULL);
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

		template<class T = KIPCRequest> const T* ReceiveRequest(const wxString& item, const void* data, size_t nSize) const
		{
			static_assert(std::is_trivially_copyable<T>::value, "T is not trivially copyable");
			wxLogInfo("Client: Receive request \"%s\"", item);

			if (nSize == sizeof(T) && !item.IsEmpty())
			{
				const KIPCRequest* baseRequest = reinterpret_cast<const KIPCRequest*>(data);
				if (baseRequest && T::GetClassName() == item)
				{
					return static_cast<const T*>(baseRequest);
				}
			}
			else
			{
				wxLogInfo("Client: Invalid request size for \"%s\"", item);
			}
			return NULL;
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