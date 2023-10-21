/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/21 18:55:01 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Server.hpp"

/* Constructors & Destructors */

	/* Public */

Server::Server(const ServerSockets::t_serverconfig &serverConfig):
	_serverSockets(ServerSockets(serverConfig))
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: parameter constructor called.";
		std::cout << WHITE;
	}

	setCommands();

	{
		const ServerSockets::Sockets	&sockets = _serverSockets.getSockets();

		for (ServerSockets::SocketsConstIt it = sockets.begin(); it != sockets.end(); it++)
			_pollFds.push_back(generatePollFd(*it));
	}

	eventLoop();
}

Server::Server(const Server &other):
	_serverSockets(other._serverSockets),
	_pollFds(other._pollFds),
	_clients(other._clients),
	_channels(other._channels)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: copy constructor called.";
		std::cout << WHITE;
	}
}

Server	&Server::operator=(const Server &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_serverSockets = other._serverSockets;
		_pollFds = other._pollFds;
		_clients = other._clients;
		_channels = other._channels;
	}

	return (*this);
}

Server::~Server(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: default destructor called.";
		std::cout << WHITE;
	}

	_pollFds.clear();
	deleteClients();
	deleteChannels();
}
	/* Protected */
	/* Private */

Server::Server(void):
	_serverSockets(ServerSockets())
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: default constructor called.";
		std::cout << WHITE;
	}

	setCommands();

	{
		const ServerSockets::Sockets	&sockets = _serverSockets.getSockets();

		for (ServerSockets::SocketsConstIt it = sockets.begin(); it != sockets.end(); it++)
			_pollFds.push_back(generatePollFd(*it));
	}
}

/* Member functions */

	/* Public */
const struct pollfd	Server::generatePollFd(const ASocket::t_socket	&serverSocket)
{
	struct pollfd	pollFd;

	memset(&pollFd, 0, sizeof(pollFd));

	pollFd.fd = serverSocket.fd;
	pollFd.events = POLLIN;

	return (pollFd);
}

void	Server::deleteClients(void)
{
	for (ClientsIterator it = _clients.begin(); it < _clients.end(); it++)
		delete *it;
	_clients.clear();
}

void	Server::deleteChannels(void)
{
	for (ChannelsIterator it = _channels.begin(); it < _channels.end(); it++)
		delete *it;
	_channels.clear();
}

	/* Protected */
	/* Private */

void	Server::eventLoop(void)
{
	const ServerSockets::Sockets	&serverSockets = _serverSockets.getSockets();

	while (g_serverExit == false)
	{
		int		activity = poll(_pollFds.data(), _pollFds.size(), -1);

		if (g_serverExit)
			continue ;
		else if (activity < 0)
		{
			deleteClients();
			deleteChannels();
			throw	std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");
		}

		size_t	i = 0;

		handleServerPollFds(serverSockets, i);
		handleClientsPollFds(serverSockets, i);
	}
}

void	Server::handleServerPollFds(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	for (; i < serverSockets.size(); i++)
	{
		const ASocket::t_socket	&serverSocket = serverSockets[i];
		struct pollfd			&pollFd = _pollFds[i];

		switch (pollFd.revents)
		{
			case POLLIN:
				pollFd.revents &= ~POLLIN;
				handleClientConnections(serverSocket);
				break ;
			default:
				break ;
		}
	}
}

void	Server::handleClientsPollFds(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	for (; i < _pollFds.size(); i++)
	{
		Client			*client = _clients[i - serverSockets.size()];
		struct pollfd	&pollFd = _pollFds[i];

		switch (pollFd.revents)
		{
			case POLLIN:
				pollFd.revents &= ~POLLIN;
				if (handleClientDataReception(client, pollFd) == CLIENT_DISCONNECTED)
				{
					pollFd.revents &= ~POLLHUP;
					handleClientDisconnections(serverSockets, i);
				}
				break ;
			case POLLHUP:
				pollFd.revents &= ~POLLHUP;
				handleClientDisconnections(serverSockets, i);
				break ;
			case POLLERR:
				break ;
			default:
				break ;
		}
	}
}

// ACT ON POLL EVENTS.

void	Server::handleClientConnections(const ServerSockets::t_socket &serverSocket)
{
	try
	{
		Client			*client = new Client(serverSocket);
		struct	pollfd	clientPollFd = client->generatePollFd();

		_clients.push_back(client);
		_pollFds.push_back(clientPollFd);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: client couldn't connect (server)." << std::endl;
	}
}

/**
 *	With a TCP socket, excess data isn't discarded but is buffered
 *	by the operating system. The subsequent recv calls will read that data.
**/
bool	Server::handleClientDataReception(Client *client, struct pollfd &pollFd)
{
	if (client->readAndStoreFdBuffer(*this, pollFd) == CLIENT_DISCONNECTED)
		return (CLIENT_DISCONNECTED);

	std::string			&clientBuffer = client->getBuffer();
	const std::string	delimiter = DELIMITER;
	size_t				pos;

	removeLeadingWhitespaces(clientBuffer);

	while ((pos = clientBuffer.find(delimiter)) != std::string::npos)
	{
		if (clientBuffer[0] == '/')
		{
			// CMD
			executeCommand(client, clientBuffer);
			clientBuffer.clear();
		}
		else
		{
			// MSG or maybe files when we'll do bonuses ?
			putMessage(clientBuffer, delimiter, pos);
		}
		removeLeadingWhitespaces(clientBuffer);
	}

	return (CLIENT_CONNECTED);
}

void	Server::handleClientDisconnections(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	ClientsIterator	clientIt = _clients.begin() + (i - serverSockets.size());

	// Should disconnect from channel(s) also.

	delete *clientIt;
	_clients.erase(clientIt);

	_pollFds.erase(_pollFds.begin() + i);
	i--;
}

void	Server::executeCommand(Client *client, std::string &clientBuffer)
{
	t_commandParams	commandParams;

	commandParams.who = client;
	commandParams.target = NULL;
	commandParams.message = NULL;

	CommandsIterator	commandIt = _commands.find(getNextWord(clientBuffer));
	if (commandIt == _commands.end())
		return ;			// error msg ? Command not found.

	(this->*commandIt->second)(client, commandParams);
}

void	Server::setCommands(void)
{
	_commands["/NICK"] = &Server::nickCommand;
	_commands["/QUIT"] = NULL;
	_commands["/JOIN"] = NULL;
	_commands["/WHOIS"] = NULL;
	_commands["/PRIVMSG"] = NULL;
	_commands["/NOTICE"] = NULL;
	_commands["/KICK"] = NULL;
	_commands["/MODE"] = NULL;
	_commands["/TOPIC"] = NULL;
	_commands["/INVITE"] = NULL;
	_commands["/WHO"] = NULL;
	_commands["/NAMES"] = NULL;
	_commands["/PART"] = NULL;
}

void	Server::nickCommand(Client *client, const t_commandParams &commandParams)
{
	(void)client;
	(void)commandParams;
	std::cout << "AAAAA" << std::endl;
}

void	Server::putMessage(std::string &clientBuffer, const std::string &delimiter, size_t &pos)
{
	std::string			message;

	if (pos >= (MSG_BUFFER_SIZE - delimiter.length()))
	{
		pos = MSG_BUFFER_SIZE - delimiter.length();
		pos = findLastWordEnd(clientBuffer, pos);
		message = clientBuffer.substr(0, pos);
		clientBuffer.erase(0, pos);
		message += delimiter;
	}
	else
	{
		pos += delimiter.length();
		message = clientBuffer.substr(0, pos);
		clientBuffer.erase(0, pos);
	}

	// TEMP
	if (message != delimiter)
		std::cout << "Client n°" << "x" << ": " << message;
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
	/* Protected */
	/* Private */

const Server::t_commandParams	Server::buildCommandParams(Client *who,
	const void *target, const char *message)
{
	t_commandParams	commandParameters;

	commandParameters.who = who;
	commandParameters.target = target;
	commandParameters.message = message;

	return (commandParameters);
}
