/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:32 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/08 01:30:32 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

/* Constructors & Destructors */

	/* Public */

Client::Client(const ASocket::t_socket &serverSocket):
	_clientSocket(ClientSocket(serverSocket)),
	_activeChannel(NULL),
	_serverPermissions(0),
	_connectionRetries(1),
	_nickname("*"),
	_username("*"),
	_hostname("*"),
	_originServername("*"),
	_realname("*")
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: parameter constructor called.";
		std::cout << WHITE;
	}
}

Client::Client(const Client &other):
	_clientSocket(other._clientSocket),
	_activeChannel(other._activeChannel),
	_serverPermissions(other._serverPermissions),
	_connectionRetries(other._connectionRetries),
	_nickname(other._nickname),
	_username(other._username),
	_hostname(other._hostname),
	_originServername(other._originServername),
	_realname(other._realname),
	_messageBuffer(other._messageBuffer)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: copy constructor called.";
		std::cout << WHITE;
	}
}

Client	&Client::operator=(const Client &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_clientSocket = other._clientSocket;
		_activeChannel = other._activeChannel;
		_serverPermissions = other._serverPermissions;
		_connectionRetries = other._connectionRetries;
		_nickname = other._nickname;
		_username = other._username;
		_hostname = other._hostname;
		_originServername = other._originServername;
		_realname = other._realname;
		_messageBuffer = other._messageBuffer;
	}

	return (*this);
}

Client::~Client(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: default destructor called.";
		std::cout << WHITE;
	}
}
	/* Protected */
	/* Private */

Client::Client(void):
	_clientSocket(ClientSocket()),
	_activeChannel(NULL),
	_serverPermissions(0),
	_connectionRetries(1),
	_nickname("*"),
	_username("*"),
	_hostname("*"),
	_originServername("*"),
	_realname("*")
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: default constructor called.";
		std::cout << WHITE;
	}
}

/* Member functions */

	/* Public */

const struct pollfd	Client::generatePollFd(void)
{
	struct pollfd	pollFd;

	memset(&pollFd, 0, sizeof(pollFd));

	pollFd.fd = _clientSocket.getSocket().fd;
	pollFd.events = POLLIN | POLLHUP | POLLERR;

	return (pollFd);
}

void	Client::closeSocketFd(void)
{
	close(_clientSocket.getSocket().fd);
}

void	Client::incrementConnectionRetries(void)
{
	_connectionRetries++;
}

void	Client::receiveMessage(const std::string &message) const
{
	send(getSocketFd(), message.c_str(), message.length(), 0);
}

void	Client::broadcastMessageToChannel(const Channel *channel,
	const std::string &message) const
{
	if (!channel)
		return ;

	std::string	response;
	Channel::Users	users = channel->getUsers();

	for (Channel::UsersIterator it = users.begin(); it != users.end(); it++)
	{
		if (this != it->client)
			it->client->receiveMessage(message);
	}
}

void	Client::joinChannel(Channel *channel)
{
	if (!channel || channel->isFull())
		return ;

	if (channel->isEmpty())
		channel->addUser(this, Channel::defaultOwnerPerms());
	else
		channel->addUser(this, Channel::defaultUserPerms());

	_joinedChannels[channel->getName()] = channel;
	_activeChannel = channel;
}

void	Client::quitChannel(Channel *channel)
{
	if (!channel)
		return ;

	channel->removeUser(this);
	_joinedChannels.erase(channel->getName());
	_activeChannel = NULL;
}

	/* Protected */
	/* Private */

/* Getters */

	/* Public */

std::string	&Client::getBuffer(void)
{
	return (_messageBuffer);
}

int	Client::getSocketFd(void) const
{
	return (_clientSocket.getSocket().fd);
}

const std::string	&Client::getNickname(void) const
{
	return (_nickname);
}

const std::string	&Client::getUsername(void) const
{
	return (_username);
}

const std::string	&Client::getHostname(void) const
{
	return (_hostname);
}

const std::string	&Client::getServername(void) const
{
	return (_originServername);
}

const std::string	&Client::getRealname(void) const
{
	return (_realname);
}

short	&Client::getConnectionRetries(void)
{
	return (_connectionRetries);
}

int	Client::getServerPermissions(void) const
{
	return (_serverPermissions);
}

Channel::Channels	&Client::getJoinedChannels(void)
{
	return (_joinedChannels);
}


Channel	*Client::getActiveChannel(void)
{
	return (_activeChannel);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */

int	Client::readAndStoreFdBuffer(Server &server, const struct pollfd &pollFd)
{
	int				readBytes = -1;
	char			recvBuffer[MSG_BUFFER_SIZE];
	memset(recvBuffer, 0, sizeof(recvBuffer));

	readBytes = recv(pollFd.fd, recvBuffer, sizeof(recvBuffer), 0);
		const int			&getSocketFd(void);
	if (readBytes < 0)
	{
		server.deleteClients();
		throw std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");
	}
	else if (readBytes == 0)
		return (CLIENT_DISCONNECTED);

	_messageBuffer.append(recvBuffer, readBytes);
	return (CLIENT_CONNECTED);
}


void	Client::setNickname(const std::string &nickname)
{
	_nickname = truncate(nickname, MAX_NICKNAME_LEN);
}

void	Client::setUsername(const std::string &username)
{
	_username = truncate(username, MAX_USERNAME_LEN);
}

void	Client::setHostname(const std::string &hostname)
{
	_hostname = hostname;
}

void	Client::setServername(const std::string &servername)
{
	_originServername = servername;
}

void	Client::setRealname(const std::string &realname)
{
	_realname = realname;
}

void	Client::setServerPermissions(const int &mask)
{
	setBits(_serverPermissions, mask);
}

void	Client::setActiveChannel(Channel *channel)
{
	_activeChannel = channel;
}

	/* Protected */
	/* Private */

/* Static */

	/* Public */

bool	Client::isClientMode(const char &mode)
{
	if (strchr(MODES_CLIENT, mode))
		return (true);
	return (false);
}

	/* Protected */
	/* Private */

int	Client::clientModeToMask(const char &mode)
{
	switch (mode)
	{
		case 'i':
			return (INVISIBLE);
		case 'w':
			return (WALLOPS);
		case 'o':
			return (OPERATOR);
		case 'x':
			return (SSL_TLS);
		case 'z':
			return (HIDE_HOSTNAME);
		default:
			return (0);
	}
}

std::string	Client::clientMaskToModes(const int &mask)
{
	std::string	modes;
	const char	*clientModes = MODES_CLIENT;

	for (size_t shift = 0; shift < strlen(clientModes); shift++)
	{
		if((mask >> shift) & 1)
			modes += clientModes[shift];
	}

	if (modes.empty())
		return ("");
	return ("+" + modes);
}
