#include "twainet_module.h"
#include "ipc_lib\connector\ipc_connector_factory.h"
#include "application\application.h"

TwainetModule::TwainetModule(const IPCObjectName& ipcName)
	: TunnelModule(ipcName, new IPCConnectorFactory<IPCConnector>(ipcName))
{
}

TwainetModule::~TwainetModule()
{
}

void TwainetModule::OnTunnelConnectFailed(const std::string& sessionId)
{
	Application::GetInstance().AddNotifycationMessage(new ConnectionFailed(this, sessionId, true));
}

void TwainetModule::OnTunnelConnected(const std::string& sessionId, TunnelConnector::TypeConnection type)
{
	Application::GetInstance().AddNotifycationMessage(new TunnelConnected(this, sessionId, type));
}

void TwainetModule::OnServerConnected()
{
	Application::GetInstance().AddNotifycationMessage(new ClientServerConnected(this, m_ownSessionId, false));
}

void TwainetModule::OnClientConnector(const std::string& sessionId)
{
	Application::GetInstance().AddNotifycationMessage(new ClientServerConnected(this, sessionId, true));
}

void TwainetModule::OnFireConnector(const std::string& moduleName)
{
	Application::GetInstance().AddNotifycationMessage(new ModuleDisconnected(this, moduleName));
}

void TwainetModule::OnConnectFailed(const std::string& moduleName)
{
	Application::GetInstance().AddNotifycationMessage(new ConnectionFailed(this, moduleName, false));
}

void TwainetModule::OnConnected(const std::string& moduleName)
{
	Application::GetInstance().AddNotifycationMessage(new ModuleConnected(this, moduleName));
}

void TwainetModule::OnMessage(const std::string& messageName, const std::vector<std::string>& path, const std::string& data)
{
	Application::GetInstance().AddNotifycationMessage(new GettingMessage(this, messageName, path, data));
}