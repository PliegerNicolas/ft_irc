/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:32 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/15 10:57:30 by nicolas          ###   ########.fr       */
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
	_modesMask(HIDE_HOSTNAME),
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
		std::cout << WHITE << std::endl;
	}
	std::cout << PURPLE << "A new client has connected to the server" << WHITE << std::endl;
}

Client::Client(const Client &other):
	_clientSocket(other._clientSocket),
	_activeChannel(other._activeChannel),
	_serverPermissions(other._serverPermissions),
	_connectionRetries(other._connectionRetries),
	_modesMask(other._modesMask),
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
		std::cout << WHITE << std::endl;
	}
	std::cout << PURPLE << "A new client has connected to the server" << WHITE << std::endl;
}

Client	&Client::operator=(const Client &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: assignment operator called.";
		std::cout << WHITE << std::endl;
	}

	if (this != &other)
	{
		_clientSocket = other._clientSocket;
		_activeChannel = other._activeChannel;
		_serverPermissions = other._serverPermissions;
		_connectionRetries = other._connectionRetries;
		_modesMask = other._modesMask;
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
		std::cout << WHITE << std::endl;
	}

	_activeChannel = NULL;
	_joinedChannels.clear();
	std::cout << BPURPLE << _nickname << PURPLE << " has left the server" << WHITE << std::endl;
}
	/* Protected */
	/* Private */

Client::Client(void):
	_clientSocket(ClientSocket()),
	_activeChannel(NULL),
	_serverPermissions(0),
	_connectionRetries(1),
	_modesMask(0),
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
		std::cout << WHITE << std::endl;
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

	for (Channel::UsersIterator it = users.begin(); it != users.end(); ++it)
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

int	Client::addClientMode(const char &mode, const std::string &argument)
{
	int	mask = Client::clientModeToMask(mode);

	(void)argument;

	if (!mask)
		return (MODE_INVALID);
	else if (areBitsSet(_modesMask, mask))
		return (MODE_UNCHANGED);
	else
		return (setBits(_modesMask, mask), MODE_CHANGED);
}

int	Client::removeClientMode(const char &mode)
{
	int	mask = Client::clientModeToMask(mode);

	if (!mask)
		return (MODE_INVALID);
	else if (areBitsNotSet(_modesMask, mask))
		return (MODE_UNCHANGED);
	else
		return (removeBits(_modesMask, mask), MODE_CHANGED);
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

const std::string	Client::getClientModes(void) const
{
	return (Client::clientMaskToModes(_modesMask));
}

int	Client::getClientModesMask(void) const
{
	return (_modesMask);
}

const std::string	Client::getPrefix(void) const
{
	std::string	prefix;

	if (areBitsSet(_modesMask, OPERATOR))
		prefix += "~";

	return (prefix);
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

void	Client::setActiveChannel(Channel *channel)
{
	_activeChannel = channel;
}



void	Client::setServerPermissions(const int &mask)
{
	setBits(_serverPermissions, mask);
}

void	Client::setClientModesMask(const int &mask)
{
	setBits(_modesMask, mask);
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

	for (size_t shift = 0; shift < strlen(clientModes); ++shift)
	{
		if((mask >> shift) & 1)
			modes += clientModes[shift];
	}

	if (modes.empty())
		return ("");
	return ("+" + modes);
}

bool	Client::isValidNickname(const std::string &nickname)
{
	if (nickname.empty() || nickname.length() > MAX_NICKNAME_LEN || nickname[0] == '#')
		return (false);
	return (true);
}
