/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSockets.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:19:49 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/15 10:59:13 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ServerSockets.hpp"

/* Constructors & Destructors */

	/* Public */
ServerSockets::ServerSockets(void):
	ASocket(),
	_hostname("localhost")
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Default constructor called.";
		std::cout << WHITE << std::endl;
	}

	// Set default server configurations
	t_serverconfig	serverConfig = buildServerConfig(AF_UNSPEC,
		SOCK_STREAM, IPPROTO_TCP, "localhost", "6667");

	launchServerSockets(serverConfig);
}

ServerSockets::ServerSockets(const t_serverconfig &serverConfig):
	ASocket(),
	_hostname(serverConfig.interface)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Parameter constructor called.";
		std::cout << WHITE << std::endl;
	}

	launchServerSockets(serverConfig);
}

ServerSockets::ServerSockets(const ServerSockets &other):
	ASocket(other),
	_sockets(other._sockets)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Copy constructor called.";
		std::cout << WHITE << std::endl;
	}
}

ServerSockets	&ServerSockets::operator=(const ServerSockets &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSockets: Assignment operator called.";
		std::cout << WHITE << std::endl;
	}

	if (this != &other)
	{
		ASocket::operator=(other);
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
		std::cout << WHITE << std::endl;
	}

	for (SocketsIt it = _sockets.begin(); it != _sockets.end(); ++it)
		close(it->fd);
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
	verifyPort(serverConfig.port);

	_hints.ai_family = serverConfig.domain;
	_hints.ai_socktype = serverConfig.service;
	_hints.ai_protocol = serverConfig.protocol;
	_hints.ai_flags = AI_PASSIVE | AI_CANONNAME;

	// Retrieve with getaddrinfo() all matching internet addresses (and more).
	{
		int status = getaddrinfo(serverConfig.interface, serverConfig.port,
			&_hints, &_addrInfo);
		if (status != 0)
			throw std::runtime_error(std::string("Error: ")
				+ gai_strerror(status) + " (socket).");
	}

	// Try to open a socket and bind it for every matching address.
	for (struct addrinfo *ai = _addrInfo; ai != NULL; ai = ai->ai_next)
	{
		t_socket		newSocket;

		newSocket.info = ai;

		// Generate socket file descriptor and verify success of operation.
		newSocket.fd = socket(newSocket.info->ai_family,
			newSocket.info->ai_socktype, newSocket.info->ai_protocol);
		handleErrors(newSocket.fd);

		_sockets.push_back(newSocket);

		// Verify we can bind to it and do if it's possible.
		handleErrors(bind(newSocket.fd, newSocket.info->ai_addr,
			newSocket.info->ai_addrlen));

		// Verify if we can listen with it and do if it's possible.
		handleErrors(listen(newSocket.fd, SOMAXCONN));
	}

	setSocketOptions();
}

void	ServerSockets::setSocketOptions(void)
{
	t_sooption	socketOptions[SERVOPTSIZE];
	size_t		i = 0;

	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_REUSEADDR, 1);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_RCVBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_SNDBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_KEEPALIVE, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_QUICKACK, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_NODELAY, 1);

	for (size_t i = 0; i < SERVOPTSIZE; ++i)
	{
		for (SocketsConstIt it = _sockets.begin(); it != _sockets.end(); ++it)
		{
			if (setsockopt(it->fd, socketOptions[i].level, socketOptions[i].option,
				&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
			{
				throw std::runtime_error("Error: couldn't set socket option (socket).");
			}

			if (fcntl(it->fd, F_SETFL, O_NONBLOCK))
			{
				throw std::runtime_error("Error: couldn't set socket option (socket).");
			}
		}
	}
}

void	ServerSockets::handleErrors(const int &statusCode)
{
	if (statusCode >= 0)
		return ;

	for (SocketsIt it = _sockets.begin(); it != _sockets.end(); ++it)
		close(it->fd);
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

const ServerSockets::Sockets	&ServerSockets::getSockets(void) const
{
	return (_sockets);
}

const std::string	&ServerSockets::getHostname(void) const
{
	return (_hostname);
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
ServerSockets::buildServerConfig(const int &domain, const int &service,
	const int &protocol, const char *interface, const char *port)
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
