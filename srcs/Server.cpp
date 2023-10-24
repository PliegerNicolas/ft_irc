/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mfaucheu <mfaucheu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/24 15:01:57 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/* ************************************************************************** */
/* *                       Constructors & Destructors                       * */
/* ************************************************************************** */

/* Public */

Server::Server(const ServerSockets::t_serverconfig &serverConfig,
	const std::string &password):
	_serverSockets(ServerSockets(serverConfig)),
	_password(password)
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
	_commands(other._commands),
	_password(other._password)
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
		_password = other._password;
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
	_serverSockets(ServerSockets()),
	_password("")
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
			putMessage(client, delimiter, pos);

		clientBuffer.erase(0, delimiter.length());
	}
	while ((pos = clientBuffer.find(delimiter)) != std::string::npos);

	if ((~(client->getConnectionRetries()) & VERIFIED)
		&& client->getConnectionRetries() >= MAX_CONNECTION_RETRIES)
	{
		std::cerr << "Error: Shesh too much retries (temp msg)." << std::endl;
		return (CLIENT_DISCONNECTED);
	}

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
	_commands["PASS"] = &Server::pass;
	_commands["NICK"] = &Server::nick;
	_commands["USER"] = &Server::user;
	_commands["JOIN"] = &Server::join;
	_commands["INVITE"] = &Server::invite;
	_commands["WHOIS"] = &Server::whois;
	_commands["PRIVMSG"] = &Server::privmsg;
	_commands["MODE"] = &Server::mode;
	_commands["KICK"] = &Server::kick;
	_commands["NOTICE"] = &Server::notice;
	_commands["TOPIC"] = &Server::topic;
	_commands["WHO"] = &Server::who;
	_commands["NAMES"] = &Server::names;
	_commands["PART"] = &Server::part;
	_commands["CAP"] = &Server::cap;
	_commands["QUIT"] = &Server::quit;
}

void	Server::executeCommand(Client *client, std::string &clientBuffer,
	const std::string &delimiter)
{
	CommandFunction				command;
	t_commandParams				commandParams;

	{
		std::string	word = getNextWord(clientBuffer, delimiter);
		if (word[0] == '/')
			word.erase(0, 1);
		capitalizeString(word);
		command = _commands.find(word)->second;
	}

	commandParams = parseCommand(client, clientBuffer, delimiter);
	(this->*command)(commandParams);
}

Server::t_commandParams	Server::parseCommand(Client *client, std::string &clientBuffer,
	const std::string &delimiter)
{
	t_commandParams				commandParams;
	std::string					word;
	std::vector<std::string>	parameters;
	std::string					message;

	while (clientBuffer.find(delimiter) > 0 && clientBuffer[0] != ':')
	{
		word = getNextWord(clientBuffer, delimiter);
		parameters.push_back(word);
	}

	if (clientBuffer[0] == ':')
	{
		size_t	pos;
		clientBuffer.erase(0, 1);
		pos = clientBuffer.find(delimiter);
		message = clientBuffer.substr(0, pos);
		clientBuffer.erase(0, pos);
	}

	return (buildCommandParams(client, parameters, message));
}

/* ************************************************************************** */
/* *                           Command Functions                            * */
/* ************************************************************************** */

void	Server::cap(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	// Unclear
	(void)commandParams;
	std::cout << "CAP command executed." << std::endl;
}

void	Server::nick(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in NICK command (temp message)." << std::endl;
		return ;
	}

	Client				*source = commandParams.source;
	const	std::string	&nickname = commandParams.arguments[0];

	if (nickname.length() >= 10)
	{
		std::cerr << "Error: nickname too long";
		std::cerr << " in NICK command (temp message)." << std::endl;
		return ;
	}

	for (ClientsIterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (nickname == (*it)->getNickname())
		{
			std::cerr << "Error: nickname already exists";
			std:: cerr << " in NICK command (temp message)." << std::endl;
			return ;
		}
	}

	source->setNickname(nickname);
	source->setServerPermissions(IDENTIFIED);
}

void	Server::user(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE)
		|| commandParams.arguments.size() != 3)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in NICK command (temp message)." << std::endl;
		return ;
	}

	// This command should set the user's nickname. Careful.
	// Nicknames should be unique to ensure accessibility !
	// Nicknames are freed on client disconnection. There is not
	// persistence.
	std::cout << "USER command executed." << std::endl;
}

void	Server::quit(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE))
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in QUIT command (temp message)." << std::endl;
		return ;
	}

	// This quits the server so destroys the affiliates client.
	// See Server::handleClientDisconnection()
	std::cout << "QUIT command executed." << std::endl;
}

void	Server::join(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in JOIN command (temp message)." << std::endl;
		return ;
	}

	// This adds the client to the channel's list and add it
	// to it's active channel.
	// If the channel doesn't exist, it creates it and
	// gives operator privileges to the creator.
	std::cout << "JOIN command executed." << std::endl;
}

void	Server::whois(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in WHOIS command (temp message)." << std::endl;
		return ;
	}

	// Gets information about an existing user :
	// nickname, name, blablabla ...
	std::cout << "WHOIS command executed." << std::endl;
}

void	Server::privmsg(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in PRIVMSG command (temp message)." << std::endl;
		return ;
	}

	// Sends a message to the target (Channel or Client).
	std::cout << "PRIVMSG command executed." << std::endl;
}

void	Server::notice(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in NOTICE command (temp message)." << std::endl;
		return ;
	}

	// Sends a server notice to a client, channel or everywhere.
	std::cout << "NOTICE command executed." << std::endl;
}

void	Server::kick(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| (commandParams.arguments.size() >= 1 && commandParams.arguments.size() <= 2))
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in KICK command (temp message)." << std::endl;
		return ;
	}

	// Kicks a target out of a channel.
	std::cout << "KICK command executed." << std::endl;
}

void	Server::mode(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| (commandParams.arguments.size() >= 1 && commandParams.arguments.size() <= 2))
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in MODE command (temp message)." << std::endl;
		return ;
	}

	// ???
	std::cout << "MODE command executed." << std::endl;
}

void	Server::topic(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in TOPIC command (temp message)." << std::endl;
		return ;
	}

	// ???
	std::cout << "TOPIC command executed." << std::endl;
}

void	Server::invite(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() != 2)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in INVITE command (temp message)." << std::endl;
		return ;
	}

	// Invites a client to a channel.
	// My current commandParams aren't adapted to this.
	std::cout << "INVITE command executed." << std::endl;
}

void	Server::who(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in WHO command (temp message)." << std::endl;
		return ;
	}

	//  List the users in a channel (names, real names, server info, status, ...)
	std::cout << "WHO command executed." << std::endl;
}

void	Server::names(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in NAMES command (temp message)." << std::endl;
		return ;
	}

	// Lists users of a channel (nicknames and status)
	std::cout << "NAMES command executed." << std::endl;
}

void	Server::part(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() != 1)
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in PART command (temp message)." << std::endl;
		return ;
	}

	// Leaves a channel. A user can be member of multiple channels at the same time
	// for persistence.
	std::cout << "PART command executed." << std::endl;
}

void	Server::pass(const t_commandParams &commandParams)
{
	if (((commandParams.mask & (SOURCE | ARGUMENTS)) != (SOURCE | ARGUMENTS))
		|| (commandParams.arguments.size() != 1))
	{
		std::cerr << "Error: invalid arguments";
		std::cerr << " in PASS command (temp message)." << std::endl;
		return ;
	}

	Client	*source = commandParams.source;

	if (areBitsSet(source->getServerPermissions(), VERIFIED))
	{
		std::cerr << "Error: already VERIFIED in PASS command (temp message)." << std::endl;
		return ;
	}
	else if (_password != commandParams.arguments[0])
	{
		std::cerr << "Error: wrong password PASS command (temp message)." << std::endl;
		source->incrementConnectionRetries();
	}
	else
		source->setServerPermissions(VERIFIED);
}

/* ************************************************************************** */
/* *                            Server utilities                            * */
/* ************************************************************************** */

void	Server::putMessage(Client *client, const std::string &delimiter, size_t &pos)
{
	std::string			&clientBuffer = client->getBuffer();

	if (verifyServerPermissions(client, VERIFIED | IDENTIFIED))
	{
		clientBuffer.clear();
		return ;
	}

	std::string			message;

	if (pos >= (MSG_BUFFER_SIZE - delimiter.length()))
	{
		pos = MSG_BUFFER_SIZE - delimiter.length();
		pos = findLastChar(clientBuffer, pos);
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
		std::cout << "Client " << client->getNickname() << ": " << message;
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

bool	Server::verifyServerPermissions(const Client *client, const int &mask)
{
	if (areBitsSet(mask, VERIFIED)
		&& areBitsNotSet(client->getServerPermissions(), VERIFIED))
	{
		std::cerr << "Error: User isn't VERIFIED. Use PASS [password]";
		std::cerr << " to verify your access permissions (temp)." << std::endl;
		return (true);
	}
	else if (areBitsSet(mask, IDENTIFIED)
		&& areBitsNotSet(client->getServerPermissions(), IDENTIFIED))
	{
		std::cerr << "Error: User isn't VERIFIED. Use PASS [password]";
		std::cerr << " to verify your access permissions (temp)." << std::endl;
		return (true);
	}
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
	std::vector<std::string> &arguments, std::string &message)
{
	t_commandParams	commandParameters;

	commandParameters.mask = 0;

	if (source)
	{
		commandParameters.mask |= SOURCE;
		commandParameters.source = source;
	}
	else
		commandParameters.source = NULL;

	if (arguments.size() > 0)
	{
		commandParameters.mask |= ARGUMENTS;
		commandParameters.arguments = arguments;
	}

	if (!message.empty())
	{
		commandParameters.mask |= MESSAGE;
		commandParameters.message = message;
	}

	return (commandParameters);
}

