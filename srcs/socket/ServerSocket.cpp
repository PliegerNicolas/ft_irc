/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:19:49 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 17:08:52 by nicolas          ###   ########.fr       */
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
		std::cout << "ServerSocket: default constructor called.";
		std::cout << WHITE;
	}

	t_soconfig	socketConfig;

	// Set configuration struct
	socketConfig = buildSocketConfig(AF_INET, SOCK_STREAM, 0, "0.0.0.0", 0);

	if (socketConfig.port < MIN_PORT || socketConfig.port > MAX_PORT)
		throw std::runtime_error("Error: port out of bounds (socket).");

	// Set sockaddr_in
	_address.sin_family = socketConfig.domain;
	_address.sin_port = htons(socketConfig.port);
	_address.sin_addr.s_addr = inet_addr(socketConfig.interface.c_str());

	if (_address.sin_port == INADDR_NONE)
		throw std::runtime_error("Error: invalid port (socket).");
	if (_address.sin_addr.s_addr == INADDR_NONE)
		throw std::runtime_error("Error: invalid IP address (socket).");

	// Build socket
	_poll.fd = socket(socketConfig.domain, socketConfig.service, socketConfig.protocol);
	ASocket::handleSocketErrors(_poll.fd);

	// Set event to which Server listens too in poll
	_poll.events = POLLIN;

	// Set socket options
	setSocketOptions();

	// bind to network
	ASocket::handleSocketErrors(bindToNetwork());

	// listen to network communication
	ASocket::handleSocketErrors(listenToNetwork());
}

ServerSocket::ServerSocket(const int &domain, const int &service, const int &protocol,
	const std::string &interface, const int &port):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: parameter constructor called.";
		std::cout << WHITE;
	}

	t_soconfig	socketConfig;

	// Set configuration struct
	socketConfig = buildSocketConfig(domain, service, protocol, interface, port);

	if (socketConfig.port < MIN_PORT || socketConfig.port > MAX_PORT)
		throw std::runtime_error("Error: port out of bounds (socket).");

	// Set sockaddr_in
	_address.sin_family = socketConfig.domain;
	_address.sin_port = htons(socketConfig.port);
	_address.sin_addr.s_addr = inet_addr(socketConfig.interface.c_str());

	if (_address.sin_port == INADDR_NONE)
		throw std::runtime_error("Error: invalid port (socket).");
	if (_address.sin_addr.s_addr == INADDR_NONE)
		throw std::runtime_error("Error: invalid IP address (socket).");

	// Build socket
	_poll.fd = socket(socketConfig.domain, socketConfig.service, socketConfig.protocol);
	ASocket::handleSocketErrors(_poll.fd);

	// Set event to which Server listens too in poll
	_poll.events = POLLIN;

	// Set socket options
	setSocketOptions();

	// bind to network
	ASocket::handleSocketErrors(bindToNetwork());

	// listen to network communication
	ASocket::handleSocketErrors(listenToNetwork());
}

ServerSocket::ServerSocket(const ServerSocket &other):
	ASocket(other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: copy constructor called.";
		std::cout << WHITE;
	}
}

ServerSocket	&ServerSocket::operator=(const ServerSocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: assignment operator called.";
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
		std::cout << "ServerSocket: default destructor called.";
		std::cout << WHITE;
	}
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

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
		if (setsockopt(getSocketFd(), socketOptions[i].level, socketOptions[i].option,
			&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
			throw std::runtime_error("Error: couldn't set socket option (socket).");
}

int	ServerSocket::bindToNetwork(void)
{
	return (bind(this->getSocketFd(), this->getAddress(), sizeof(*this->getAddress())));
}

int	ServerSocket::listenToNetwork(void)
{
	return (listen(this->getSocketFd(), SOMAXCONN));
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
