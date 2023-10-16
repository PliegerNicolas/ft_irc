/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:19:49 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 20:32:51 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ServerSocket.hpp"

/* Constructors & Destructors */

	/* Public */
ServerSocket::ServerSocket(void):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: Default constructor called.";
		std::cout << WHITE;
	}

	// Set default server configurations
	t_serverconfig	serverConfig = buildServerConfig(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
		"127.0.0.1", "6667");

	launchServerSockets(serverConfig);
}

ServerSocket::ServerSocket(const t_serverconfig &serverConfig):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: Parameter constructor called.";
		std::cout << WHITE;
	}

	launchServerSockets(serverConfig);
}

ServerSocket::ServerSocket(const ServerSocket &other):
	ASocket(other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: Copy constructor called.";
		std::cout << WHITE;
	}
}

ServerSocket	&ServerSocket::operator=(const ServerSocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: Assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		ASocket::operator=(other);
	}

	return (*this);
}

ServerSocket::~ServerSocket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: Default destructor called.";
		std::cout << WHITE;
	}
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	ServerSocket::launchServerSockets(const t_serverconfig &serverConfig)
{
	// Verify if passed port is valid.
	{
		std::istringstream	iss(serverConfig.port);
		int	port;

		if (!(iss >> port) || !iss.eof())
			throw std::runtime_error("Error: Passed port should be an integer (socket).");
		if (port < MIN_PORT || port > MAX_PORT)
			throw std::runtime_error("Error: Port out of bounds (socket).");
	}

	// Retrieve with getaddrinfo() all matching internet addresses (and more).
	{
		_hints.ai_family = serverConfig.domain;
		_hints.ai_socktype = serverConfig.service;
		_hints.ai_flags = AI_PASSIVE | AI_CANONNAME;

		int	status = getaddrinfo(serverConfig.interface, serverConfig.port, &_hints, &_addrInfo);
		if (status != 0)
			throw std::runtime_error(std::string("Error: ") + gai_strerror(status) + " (socket).");
	}

	// Try to open a socket and bind it for every matching address.
	for (struct addrinfo *af = _addrInfo; af != NULL; af = af->ai_next)
	{
		struct pollfd	newPollFd;

		// Generate pollFd (create socket and set POLL events to listen too).
		newPollFd.fd = socket(af->ai_family, af->ai_socktype, af->ai_protocol);
		newPollFd.events = POLLIN | POLLHUP | POLLERR;

		// Save the PollFd
		_pollFds.push_back(newPollFd);

		// Verify socket has correctly been created and set socket options.
		ASocket::handleSocketErrors(newPollFd.fd);
		setSocketOptions();

		// Verify we can bind to it.
		ASocket::handleSocketErrors(bind(newPollFd.fd, af->ai_addr, af->ai_addrlen));
	}
}

void	ServerSocket::setSocketOptions(void)
{
	t_sooption	socketOptions[SERVOPTSIZE];
	size_t		i = 0;

	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_REUSEADDR, 1);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_RCVBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_SNDBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_KEEPALIVE, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_QUICKACK, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_NODELAY, 1);

	for (i = 0; i < SERVOPTSIZE; i++)
		for (PollFdsConstIt it = _pollFds.begin(); it != _pollFds.end(); it++)
			if (setsockopt(it->fd, socketOptions[i].level, socketOptions[i].option,
				&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
				throw std::runtime_error("Error: couldn't set socket option (socket).");
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */

/* Static */

	/* Public */

const ServerSocket::t_serverconfig
ServerSocket::buildServerConfig(const int &domain, const int &service, const int &protocol,
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

