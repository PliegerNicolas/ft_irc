/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSockets.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:19:49 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/17 01:37:50 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ServerSockets.hpp"

/* Constructors & Destructors */

	/* Public */
ServerSockets::ServerSockets(void):
	ASockets()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Default constructor called.";
		std::cout << WHITE;
	}

	// Set default server configurations
	t_serverconfig	serverConfig = buildServerConfig(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
		"127.0.0.1", "6667");

	launchServerSockets(serverConfig);
}

ServerSockets::ServerSockets(const t_serverconfig &serverConfig):
	ASockets()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Parameter constructor called.";
		std::cout << WHITE;
	}

	launchServerSockets(serverConfig);
}

ServerSockets::ServerSockets(const ServerSockets &other):
	ASockets(other),
	_sockets(other._sockets)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Copy constructor called.";
		std::cout << WHITE;
	}
}

ServerSockets	&ServerSockets::operator=(const ServerSockets &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		ASockets::operator=(other);
		_sockets = other._sockets;
	}

	return (*this);
}

ServerSockets::~ServerSockets(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Default destructor called.";
		std::cout << WHITE;
	}

	for (SocketsIt it = _sockets.begin(); it != _sockets.end(); it++)
		close(it->first);
	_sockets.clear();
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	ServerSockets::launchServerSockets(const t_serverconfig &serverConfig)
{
	struct addrinfo	hints;
	struct addrinfo	*addrInfo;

	verifyPort(serverConfig.port);

	memset(&hints, 0, sizeof(hints));
	memset(&addrInfo, 0, sizeof(addrInfo));

	hints.ai_family = serverConfig.domain;
	hints.ai_socktype = serverConfig.service;
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;

	// Retrieve with getaddrinfo() all matching internet addresses (and more).
	{
		int status = getaddrinfo(serverConfig.interface, serverConfig.port, &hints, &addrInfo);
		if (status != 0)
			throw std::runtime_error(std::string("Error: ") + gai_strerror(status) + " (socket).");
	}

	// Try to open a socket and bind it for every matching address.
	for (struct addrinfo *ai = addrInfo; ai != NULL; ai = ai->ai_next)
	{
		int				socketFd;
		struct addrinfo	current = *ai;
		current.ai_next = NULL;

		// Generate socket file descriptor.
		socketFd = socket(current.ai_family, current.ai_socktype, current.ai_protocol);
		handleServerErrors(socketFd, addrInfo);
		setSocketOptions();

		_sockets.push_back(ASockets::SocketPair(socketFd, current));

		// Verify we can bind to it.
		handleServerErrors(bind(socketFd, current.ai_addr, current.ai_addrlen), addrInfo);
	}

	if (addrInfo)
		freeaddrinfo(addrInfo);
}

void	ServerSockets::setSocketOptions(void)
{
	t_sooption	socketOptions[SERVOPTSIZE];
	size_t		i = 0;

	socketOptions[i++] = ASockets::buildSocketOption(SOL_SOCKET, SO_REUSEADDR, 1);
	socketOptions[i++] = ASockets::buildSocketOption(SOL_SOCKET, SO_RCVBUF, 8192);
	socketOptions[i++] = ASockets::buildSocketOption(SOL_SOCKET, SO_SNDBUF, 8192);
	socketOptions[i++] = ASockets::buildSocketOption(SOL_SOCKET, SO_KEEPALIVE, 1);
	socketOptions[i++] = ASockets::buildSocketOption(IPPROTO_TCP, TCP_QUICKACK, 1);
	socketOptions[i++] = ASockets::buildSocketOption(IPPROTO_TCP, TCP_NODELAY, 1);

	for (i = 0; i < SERVOPTSIZE; i++)
		for (SocketsConstIt it = _sockets.begin(); it != _sockets.end(); it++)
			if (setsockopt(it->first, socketOptions[i].level, socketOptions[i].option,
				&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
				throw std::runtime_error("Error: couldn't set socket option (socket).");
}

void	ServerSockets::handleServerErrors(const int &statusCode, struct addrinfo *addrInfo)
{
	if (statusCode >= 0)
		return ;

	if (addrInfo)
		freeaddrinfo(addrInfo);
	for (SocketsIt it = _sockets.begin(); it != _sockets.end(); it++)
		close(it->first);
	_sockets.clear();

	int					errCode = errno;
	std::ostringstream	errorMessage;

	errorMessage << "Error: " << strerror(errCode) << " (socket).";
	throw std::runtime_error(errorMessage.str());
}

void	ServerSockets::verifyPort(const char *strPort)
{
	std::istringstream	iss(strPort);
	int					port;

	if (!(iss >> port) || !iss.eof())
		throw std::runtime_error("Error: Passed port should be an integer (server).");
	if (port < MIN_PORT || port > MAX_PORT)
		throw std::runtime_error("Error: Port out of bounds (server).");
}

/* Getters */

	/* Public */

const ASockets::Sockets	&ServerSockets::getSockets(void) const
{
	return (_sockets);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */

/* Static */

	/* Public */

const ServerSockets::t_serverconfig
ServerSockets::buildServerConfig(const int &domain, const int &service, const int &protocol,
	const char *interface, const char *port)
{
	t_serverconfig	serverConfig;

	serverConfig.domain = domain;
	serverConfig.service = service;
	serverConfig.protocol = protocol;

	serverConfig.interface = interface;
	serverConfig.port = port;

	return (serverConfig);
}

	/* Protected */
	/* Private */

