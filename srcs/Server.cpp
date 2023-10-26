/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mfaucheu <mfaucheu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/26 18:44:51 by nicolas          ###   ########.fr       */
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
	_commands(other._commands),
	_pollFds(other._pollFds),
	_clients(other._clients),
	_channels(other._channels),
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
		_commands = other._commands;
		_pollFds = other._pollFds;
		_clients = other._clients;
		_channels = other._channels;
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

	_commands.clear();
	_pollFds.clear();
	deleteClients();
	deleteChannels();
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
	for (Client::ClientsIterator it = _clients.begin(); it != _clients.end(); it++)
	{
		(*it)->closeSocketFd();
		delete *it;
	}
	_clients.clear();
}

void	Server::deleteChannels(void)
{
	for (Channel::ChannelsIterator it = _channels.begin(); it != _channels.end(); it++)
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
		{
			try
			{
				executeCommand(client, &pollFd, clientBuffer, delimiter);
			}
			catch (const std::exception &e)
			{
				std::cerr << e.what() << std::endl;
			}
		}
		else
			putMessage(client, delimiter, pos);

		clientBuffer.erase(0, delimiter.length());
	}
	while ((pos = clientBuffer.find(delimiter)) != std::string::npos);

	if (areBitsSet(pollFd.revents, POLLHUP))
		return (CLIENT_DISCONNECTED);

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
	Client::ClientsIterator	clientIt = _clients.begin() + (i - serverSockets.size());

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

void	Server::executeCommand(Client *client, struct pollfd *pollFd,
	std::string &clientBuffer, const std::string &delimiter)
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

	commandParams = parseCommand(client, pollFd, clientBuffer, delimiter);
	(this->*command)(commandParams);
}

Server::t_commandParams	Server::parseCommand(Client *client, struct pollfd *pollFd,
	std::string &clientBuffer, const std::string &delimiter)
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

	return (buildCommandParams(client, pollFd, parameters, message));
}

/* ************************************************************************** */
/* *                           Command Functions                            * */
/* ************************************************************************** */

void	Server::cap(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");

	std::string	subcommand = commandParams.arguments[0];

	if (subcommand == "LS")
		serverResponse(commandParams.source, ERR_CANTLOADMODULE,
			"CAP LS", "Capability negotiation is not supported");
	else if (subcommand == "REQ")
		serverResponse(commandParams.source, ERR_CANTLOADMODULE,
			"CAP REQ", "Capability negotiation is not supported");
	else if (subcommand == "ACK")
		serverResponse(commandParams.source, ERR_CANTLOADMODULE,
			"CAP ACK", "Capability negotiation is not supported");
	else if (subcommand == "NAK")
		serverResponse(commandParams.source, ERR_CANTLOADMODULE,
			"CAP NAK", "Capability negotiation is not supported");
	else if (subcommand == "END")
		serverResponse(commandParams.source, RPL_ENDOFNAMES,
			"CAP END", "End of CAP command");
	else
		serverResponse(commandParams.source, ERR_UNKNOWNCOMMAND,
			"Unknown subcommand", subcommand);
}

void	Server::nick(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client				*source = commandParams.source;
	const	std::string	&nickname = commandParams.arguments[0];

	if (nickname.length() >= 10 || nickname[0] == '#')
		serverResponse(commandParams.source, ERR_ERRONEUSNICKNAME, nickname,
			"Erroneous Nickname");

	for (Client::ClientsIterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (nickname == (*it)->getNickname())
			serverResponse(commandParams.source, ERR_NICKNAMEINUSE, nickname,
				"Nickname is already in use");
	}

	source->setNickname(nickname);
	source->setServerPermissions(IDENTIFIED);
	serverResponse(source, RPL_WELCOME, "", "You are now known as " + nickname);
}

void	Server::user(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE)
		|| commandParams.arguments.size() < 3)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 3)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// This command should set the user's nickname. Careful.
	// Nicknames should be unique to ensure accessibility !
	// Nicknames are freed on client disconnection. There is not
	// persistence.
	//std::cout << "USER command executed." << std::endl;
	serverResponse(commandParams.source, RPL_WELCOME, "", "TEMP");
}

void	Server::quit(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | POLLFD))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 0)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// clear necessary data if needed ? No response expected.

	commandParams.pollFd->revents |= POLLHUP;
}

void	Server::join(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client		*source = commandParams.source;
	std::string	channelName = commandParams.arguments[0];

	if (channelName[0] != '#')
		serverResponse(source, ERR_NOSUCHCHANNEL, channelName, "No such channel");

	Channel						*channel;
	Channel::ChannelsIterator	itChannel = _channels.find(channelName);

	if (itChannel != _channels.end())
	{
		channel = itChannel->second;
		if (channel->isFull())
			serverResponse(source, ERR_CHANNELISFULL, channelName, "Channel is full");
		else if (source->getActiveChannel() == channel)
			serverResponse(source, ERR_USERONCHANNEL, channelName, "Is already on channel");
		else
			channel->addUser(source, channel->getUserPerms());
	}
	else
	{
		channel = new Channel(channelName, source);
		_channels[channelName] = channel;
	}

	source->setActiveChannel(channel);
	source->addToJoinedChannels(channel);

	// TEMP
	source->receiveMessage(":" + source->getNickname() + " JOIN " + channelName);
	serverResponse(source, RPL_TOPIC, channelName, "topic"); // get topic
	serverResponse(source, RPL_TOPIC, channelName, "topic"); // get topic
	serverResponse(source, RPL_NAMREPLY, "= " + channelName, "usr1 user2 user3"); // get users list
	serverResponse(source, RPL_ENDOFNAMES, channelName, "End of /NAMES list");
}

void	Server::whois(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Gets information about an existing user :
	// nickname, name, blablabla ...
	std::cout << "WHOIS command executed." << std::endl;
}

void	Server::privmsg(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE))
		serverResponse(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Sends a message to the target (Channel or Client).
	std::cout << "PRIVMSG command executed." << std::endl;
}

void	Server::notice(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE))
		serverResponse(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Sends a server notice to a client, channel or everywhere.
	std::cout << "NOTICE command executed." << std::endl;
}

void	Server::kick(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Channel::User	*sourceUser = NULL;
	Channel::User	*targetUser = NULL;
	Channel			*targetChannel = NULL;

	if (commandParams.arguments.size() == 1)
	{
		if (commandParams.arguments[0][0] != '#')
		{
			targetChannel = commandParams.source->getActiveChannel();
			targetUser = targetChannel->getUser(commandParams.arguments[0]);
			sourceUser = targetChannel->getUser(commandParams.source->getNickname());
		}
	}
	else if (commandParams.arguments.size() == 2)
	{
		if (commandParams.arguments[0][0] == '#' && commandParams.arguments[1][0] != '#')
		{
			std::string	channelName = commandParams.arguments[0];
			channelName.erase(0, 1);

			Channels			&joinedChannels = commandParams.source->getJoinedChannels();
			ChannelsIterator	it = joinedChannels.find(channelName);

			if (it != joinedChannels.end())
			{
				targetChannel = it->second;
				targetUser = targetChannel->getUser(commandParams.arguments[1]);
				sourceUser = targetChannel->getUser(commandParams.source->getNickname());
			}
		}
	}

	if (targetChannel == NULL)
		serverResponse(commandParams.source, ERR_NOSUCHCHANNEL, "", "No such channel");
	else if (targetUser == NULL)
		serverResponse(commandParams.source, ERR_NOSUCHNICK, "", "No such nickname");
	else if (areBitsNotSet(sourceUser->permissionsMask, Channel::KICK) || sourceUser == NULL)
		serverResponse(commandParams.source, ERR_CHANOPRIVSNEEDED,
			"", "You're not a channel operator");

	targetChannel->removeUser(targetUser->client);

	std::string	response;
	response = ":" + sourceUser->client->getNickname();
	response += " KICK";
	response += " " + targetChannel->getName();
	response += " " + targetUser->client->getNickname();
	if (areBitsSet(commandParams.mask, MESSAGE))
		response += " :" + commandParams.message;

	targetUser->client->receiveMessage(response + DELIMITER);
}

void	Server::mode(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// ???
	std::cout << "MODE command executed." << std::endl;
}

void	Server::topic(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// ???
	std::cout << "TOPIC command executed." << std::endl;
}

void	Server::invite(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() < 2)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Invites a client to a channel.
	// My current commandParams aren't adapted to this.
	std::cout << "INVITE command executed." << std::endl;
}

void	Server::who(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	//  List the users in a channel (names, real names, server info, status, ...)
	std::cout << "WHO command executed." << std::endl;
}

void	Server::names(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Lists users of a channel (nicknames and status)
	std::cout << "NAMES command executed." << std::endl;
}

void	Server::part(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Leaves a channel. A user can be member of multiple channels at the same time
	// for persistence.
	std::cout << "PART command executed." << std::endl;
}

void	Server::pass(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		serverResponse(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client	*source = commandParams.source;

	if (areBitsSet(source->getServerPermissions(), VERIFIED))
		serverResponse(source, ERR_ALREADYREGISTERED, "",
			"You're already registered");
	else if (source->getConnectionRetries() >= MAX_CONNECTION_RETRIES)
	{
		serverResponse(source, ERR_PASSWDMISMATCH, "",
			"Too many password attempts. Access denied");
		// FORCE DISCONNECTION HERE
	}
	else if (_password != commandParams.arguments[0])
	{
		source->incrementConnectionRetries();
		serverResponse(source, ERR_PASSWDMISMATCH, "",
			"Password incorrect. Access denied");

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
	{
		message = client->getNickname() + ": " + message;
		client->broadcastMessageToChannel(message);
	}
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
		serverResponse(client, ERR_PASSWDMISMATCH, "", "You have not verified");
		return (true);
	}
	else if (areBitsSet(mask, IDENTIFIED)
		&& areBitsNotSet(client->getServerPermissions(), IDENTIFIED))
	{
		serverResponse(client, ERR_NOTREGISTERED, "", "You have not registered");
		return (true);
	}
	return (false);
}

void	Server::serverResponse(const Client *client, const std::string &code,
	const std::string &parameters, const std::string &trailing)
{
	std::string	response;
	std::string	targetNickname = client->getNickname();

	response = ":" + _serverSockets.getHostname();

	response += " " + code;

	if (targetNickname.empty())
		response += " *";
	else
		response += " " + targetNickname;

	if (!parameters.empty())
		response += " " + parameters;

	if (!trailing.empty())
		response += " :" + trailing;

	client->receiveMessage(response + DELIMITER);
	throw std::runtime_error(response);
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

Server::t_commandParams	Server::buildCommandParams(Client *source, struct pollfd *pollFd,
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

	if (pollFd)
	{
		commandParameters.mask |= POLLFD;
		commandParameters.pollFd = pollFd;
	}
	else
		commandParameters.pollFd = NULL;

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

