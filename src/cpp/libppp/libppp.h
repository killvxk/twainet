#ifndef LIBPPP_H
#define LIBPPP_H

#ifdef LIBPPP_EXPORT
	#define EXPORT_FUNC __declspec(dllexport)
	#define STDCALL		__stdcall
#else
	#define EXPORT_FUNC typedef
	#define STDCALL		__stdcall *
#endif // LIBPPP_EXPORT

extern "C"
{
	EXPORT_FUNC int (STDCALL CreatePPPSocket)(const char* sessionId);
	EXPORT_FUNC bool (STDCALL CreatePPPTunnel)(int socket);
	EXPORT_FUNC bool (STDCALL DestroyPPPTunnel)(int socket);
	EXPORT_FUNC int (STDCALL SendPPPData)(int socket, const char* data, int len);
	EXPORT_FUNC int (STDCALL RecvPPPData)(int socket, char* data, int len);
};

#endif/*LIBPPP_H*/