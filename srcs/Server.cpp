/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/23 11:57:49 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Server.hpp"

/* ************************************************************************** */
/* *                       Constructors & Destructors                       * */
/* ************************************************************************** */

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
	_channels(other._channels),
	_commands(other._commands)
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
		_commands = other._commands;
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
	_commands.clear();
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

/* ************************************************************************** */
/* *                            Member Functions                            * */
/* ************************************************************************** */

/* Public */

void	Server::deleteClients(void)
{
	for (ClientsIterator it = _clients.begin(); it != _clients.end(); it++)
	{
		(*it)->closeSocketFd();
		delete *it;
	}
	_clients.clear();
}

void	Server::deleteChannels(void)
{
	for (ChannelsIterator it = _channels.begin(); it != _channels.end(); it++)
		delete it->second;
	_channels.clear();
}

const struct pollfd	Server::generatePollFd(const ASocket::t_socket	&serverSocket)
{
	struct pollfd	pollFd;

	memset(&pollFd, 0, sizeof(pollFd));

	pollFd.fd = serverSocket.fd;
	pollFd.events = POLLIN;

	return (pollFd);
}

/* Protected */
/* Private */

/* ************************************************************************** */
/* *                               Event loop                               * */
/* ************************************************************************** */

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

/* ************************************************************************** */
/* *                          Server interactions                           * */
/* ************************************************************************** */

bool	Server::handleClientDataReception(Client *client, struct pollfd &pollFd)
{
	if (client->readAndStoreFdBuffer(*this, pollFd) == CLIENT_DISCONNECTED)
		return (CLIENT_DISCONNECTED);

	std::string			&clientBuffer = client->getBuffer();
	const std::string	delimiter = DELIMITER;
	size_t				pos;

	removeLeadingWhitespaces(clientBuffer, delimiter);
	pos = clientBuffer.find(delimiter);

	if (pos == std::string::npos)
		return (CLIENT_CONNECTED);

	do
	{
		if (isCommand(clientBuffer))
			executeCommand(client, clientBuffer, delimiter);
		else
			putMessage(clientBuffer, delimiter, pos);

		clientBuffer.erase(0, delimiter.length());
	}
	while ((pos = clientBuffer.find(delimiter)) != std::string::npos);

	return (CLIENT_CONNECTED);
}

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

void	Server::handleClientDisconnections(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	ClientsIterator	clientIt = _clients.begin() + (i - serverSockets.size());

	// Should disconnect from channel(s) also. This transmits privileges if no one has them.

	delete *clientIt;
	_clients.erase(clientIt);

	_pollFds.erase(_pollFds.begin() + i);
	i--;
}

/* ************************************************************************** */
/* *                                Commands                                * */
/* ************************************************************************** */

void	Server::setCommands(void)
{
	_commands["NICK"] = &Server::nick;
	_commands["QUIT"] = &Server::quit;
	_commands["JOIN"] = &Server::join;
	_commands["WHOIS"] = &Server::whois;
	_commands["PRIVMSG"] = &Server::privmsg;
	_commands["NOTICE"] = &Server::notice;
	_commands["KICK"] = &Server::kick;
	_commands["MODE"] = &Server::mode;
	_commands["TOPIC"] = &Server::topic;
	_commands["INVITE"] = &Server::invite;
	_commands["WHO"] = &Server::who;
	_commands["NAMES"] = &Server::names;
	_commands["PART"] = &Server::part;
	_commands["PASS"] = &Server::password;
}

void	Server::executeCommand(Client *client, std::string &clientBuffer,
	const std::string &delimiter)
{
	CommandFunction				command;
	std::string					word;
	std::vector<std::string>	parameters;
	const char					*message = NULL;
	t_commandParams				commandParams;

	word = getNextWord(clientBuffer, delimiter);
	if (word[0] == '/')
		word.erase(0, 1);
	capitalizeString(word);
	command = _commands.find(word)->second;

	while (clientBuffer.find(delimiter) > 0 || clientBuffer[0] == ':')
	{
		word = getNextWord(clientBuffer, delimiter);

		parameters.push_back(word);
	}

	if (clientBuffer[0] == ':')
	{
		size_t	pos;
		clientBuffer.erase(0, 1);
		pos = clientBuffer.find(delimiter);
		message = clientBuffer.substr(0, pos).c_str();
		clientBuffer.erase(0, pos);
	}

	commandParams = buildCommandParams(client, parameters, message);
	(this->*command)(commandParams);
}

void	Server::nick(const t_commandParams &commandParams)
{
	// Should take only one argument (new nickname) but no message.
	(void)commandParams;
	std::cout << "NICK command executed." << std::endl;

	// This command should set the user's nickname. Careful.
	// Nicknames should be unique to ensure accessibility !
	// Nicknames are freed on client disconnection. There is not
	// persistence.
}

void	Server::quit(const t_commandParams &commandParams)
{
	// Should not take arguments nor message.
	(void)commandParams;
	std::cout << "QUIT command executed." << std::endl;
	// This quits the server so destroys the affiliates client.
	// See Server::handleClientDisconnection()
}

void	Server::join(const t_commandParams &commandParams)
{
	// Should only take one argument (channel) but no message ?/.
	(void)commandParams;
	std::cout << "JOIN command executed." << std::endl;
	// This adds the client to the channel's list and add it
	// to it's active channel.
	// If the channel doesn't exist, it creates it and
	// gives operator privileges to the creator.
}

void	Server::whois(const t_commandParams &commandParams)
{
	// Should only take one argument (client name) but no message.
	(void)commandParams;
	std::cout << "WHOIS command executed." << std::endl;
	// Gets information about an existing user :
	// nickname, name, blablabla ...
}

void	Server::privmsg(const t_commandParams &commandParams)
{
	// Should only take one argument (client name or channel name) AND a message.
	(void)commandParams;
	std::cout << "PRIVMSG command executed." << std::endl;
	// Sends a message to the target (Channel or Client).
}

void	Server::notice(const t_commandParams &commandParams)
{
	// Should only take one argument (client name or channel name) AND a message.
	(void)commandParams;
	std::cout << "NOTICE command executed." << std::endl;
	// Sends a server notice to a client, channel or everywhere.
}

void	Server::kick(const t_commandParams &commandParams)
{
	// Should take 1 or 2 arguments (channel name + client name) AND a message (optional).
	(void)commandParams;
	std::cout << "KICK command executed." << std::endl;
	// Kicks a target out of a channel.
}

void	Server::mode(const t_commandParams &commandParams)
{
	// Should take 1 or 2 arguments (channel name + mode option(s)) but no message.
	(void)commandParams;
	std::cout << "MODE command executed." << std::endl;
}

void	Server::topic(const t_commandParams &commandParams)
{
	// Should take 1 argument (channel name) and a message.
	(void)commandParams;
	std::cout << "TOPIC command executed." << std::endl;
}

void	Server::invite(const t_commandParams &commandParams)
{
	// Should take 2 argument (client name + channel name) but no message.
	(void)commandParams;
	std::cout << "INVITE command executed." << std::endl;
	// Invites a client to a channel.
	// My current commandParams aren't adapted to this.
}

void	Server::who(const t_commandParams &commandParams)
{
	// Should take 1 argument (channel name) but no message.
	(void)commandParams;
	std::cout << "WHO command executed." << std::endl;
	//  List the users in a channel (names, real names, server info, status, ...)
}

void	Server::names(const t_commandParams &commandParams)
{
	// Should take 1 argument (channel name) but no message.
	(void)commandParams;
	std::cout << "NAMES command executed." << std::endl;
	// Lists users of a channel (nicknames and status)
}

void	Server::part(const t_commandParams &commandParams)
{
	// Should take 1 argument (channel name) but no message.
	(void)commandParams;
	std::cout << "PART command executed." << std::endl;
	// Leaves a channel. A user can be member of multiple channels at the same time
	// for persistence.
}

void	Server::password(const t_commandParams &commandParams)
{
	// Should take 1 argument (server password) but no message.
	// Maybe should take 2 arguments (channel_name + channel password) but no message.
	(void)commandParams;
	std::cout << "PASS command executed." << std::endl;
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
		pos = clientBuffer.find(delimiter);
		message = clientBuffer.substr(0, pos);
		clientBuffer.erase(0, pos);
		message += delimiter;
	}

	// TEMP
	if (message != delimiter)
		std::cout << "Client n°" << "x" << ": " << message;
}

bool	Server::isCommand(const std::string &clientBuffer)
{
	size_t		pos = 0;
	std::string	command;

	while (clientBuffer[pos] && !isspace(clientBuffer[pos]))
		pos++;

	command = clientBuffer.substr(0, pos);

	if (command[0] == '/')
		command.erase(0, 1);
	capitalizeString(command);

	CommandsIterator commandIt = _commands.find(command);

	if (commandIt != _commands.end())
		return (true);
	return (false);
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

Server::t_commandParams	Server::buildCommandParams(Client *source,
	std::vector<std::string> &arguments, const char *message)
{
	t_commandParams	commandParameters;
	memset(&commandParameters, 0, sizeof(commandParameters));

	if (source)
	{
		commandParameters.mask |= SOURCE;
		commandParameters.source = source;
	}

	if (arguments.size() != 0)
	{
		commandParameters.mask |= ARGUMENTS;
		commandParameters.arguments = arguments;
	}

	if (message)
	{
		commandParameters.mask |= MESSAGE;
		commandParameters.message = message;
	}

	return (commandParameters);
}
