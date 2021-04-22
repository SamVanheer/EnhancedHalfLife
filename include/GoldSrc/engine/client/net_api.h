//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "netadr.h"

constexpr int NETAPI_REQUEST_SERVERLIST = 0;  //!< Doesn't need a remote address
constexpr int NETAPI_REQUEST_PING = 1;
constexpr int NETAPI_REQUEST_RULES = 2;
constexpr int NETAPI_REQUEST_PLAYERS = 3;
constexpr int NETAPI_REQUEST_DETAILS = 4;

/**
*	@brief Set this flag for things like broadcast requests, etc.
*	where the engine should not kill the request hook after receiving the first response
*/
constexpr int FNETAPI_MULTIPLE_RESPONSE = 1 << 0;

typedef void (*net_api_response_func_t) (struct net_response_t* response);

constexpr int NET_SUCCESS = 0;
constexpr int NET_ERROR_TIMEOUT = 1 << 0;
constexpr int NET_ERROR_PROTO_UNSUPPORTED = 1 << 1;
constexpr int NET_ERROR_UNDEFINED = 1 << 2;

struct net_adrlist_t
{
	net_adrlist_t* next;
	netadr_t remote_address;
};

struct net_response_t
{
	/**
	*	@brief NET_SUCCESS or an error code
	*/
	int			error;

	/**
	*	@brief Context ID
	*/
	int			context;
	int			type;

	/**
	*	@brief Server that is responding to the request
	*/
	netadr_t	remote_address;

	/**
	*	@brief Response RTT ping time
	*/
	double		ping;
	/**
	*	@brief Key/Value pair string ( separated by backlash \ characters )
	*	WARNING:  You must copy this buffer in the callback function,
	*	because it is freed by the engine right after the call!!!!
	*	ALSO:  For NETAPI_REQUEST_SERVERLIST requests, this will be a pointer to a linked list of net_adrlist_t's
	*/
	void* response;
};

struct net_status_t
{
	/**
	*	@brief Connected to remote server?  1 == yes, 0 otherwise
	*/
	int			connected;
	/**
	*	@brief Client's IP address
	*/
	netadr_t	local_address;

	/**
	*	@brief Address of remote server
	*/
	netadr_t	remote_address;

	/**
	*	@brief Packet Loss ( as a percentage )
	*/
	int			packet_loss;

	/**
	*	@brief Latency, in seconds ( multiply by 1000.0 to get milliseconds )
	*/
	double		latency;

	/**
	*	@brief Connection time, in seconds
	*/
	double		connection_time;

	/**
	*	@brief Rate setting ( for incoming data )
	*/
	double		rate;
};

struct net_api_t
{
	void		(*InitNetworking)();
	void		(*Status) (net_status_t* status);
	void		(*SendRequest) (int context, int request, int flags, double timeout, netadr_t* remote_address, net_api_response_func_t response);
	void		(*CancelRequest) (int context);
	void		(*CancelAllRequests) ();
	char* (*AdrToString) (netadr_t* a);
	int			(*CompareAdr) (netadr_t* a, netadr_t* b);
	int			(*StringToAdr) (char* s, netadr_t* a);
	const char* (*ValueForKey) (const char* s, const char* key);
	void		(*RemoveKey) (char* s, const char* key);
	void		(*SetValueForKey) (char* s, const char* key, const char* value, int maxsize);
};
