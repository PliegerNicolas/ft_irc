/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/30 21:38:00 by hania            ###   ########.fr       */
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
		try
		{
			if (!isCommand(clientBuffer))
			{
				Channel	*channel = client->getActiveChannel();
				if (!channel)
				{
					clientBuffer.clear();
					errCommand(client, ERR_NOTONCHANNEL, "", "You are not on a channel");
				}
				clientBuffer = "/PRIVMSG " + channel->getName() + " :" + clientBuffer;
			}
			executeCommand(client, &pollFd, clientBuffer, delimiter);
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}
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

	size_t	pos = clientBuffer.find(delimiter);

	if (clientBuffer[0] == ':')
	{
		message = clientBuffer.substr(0, pos);
	}

	clientBuffer.erase(0, pos + delimiter.length());

	return (buildCommandParams(client, pollFd, parameters, message));
}

/* ************************************************************************** */
/* *                           Command Functions                            * */
/* ************************************************************************** */

/**
 *	CAP and it's subcommands communicate capabilities of server.
 *	No specific capabilities are available on our Server -yet-.
**/
void	Server::cap(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");

	const Client	*source = commandParams.source;
	std::string		subcommand = commandParams.arguments[0];

	if (subcommand == "LS")
		source->receiveMessage(getServerResponse(source, ERR_CANTLOADMODULE,
			"CAP LS", "Capability negotiation is not supported"));
	else if (subcommand == "REQ")
		source->receiveMessage(getServerResponse(source, ERR_CANTLOADMODULE,
			"CAP REQ", "Capability negotiation is not supported"));
	else if (subcommand == "ACK")
		source->receiveMessage(getServerResponse(source, ERR_CANTLOADMODULE,
			"CAP ACK", "Capability negotiation is not supported"));
	else if (subcommand == "NAK")
		source->receiveMessage(getServerResponse(source, ERR_CANTLOADMODULE,
			"CAP NAK", "Capability negotiation is not supported"));
	else if (subcommand == "END")
		source->receiveMessage(getServerResponse(source, RPL_ENDOFNAMES,
			"CAP END", "End of CAP command"));
	else
		source->receiveMessage(getServerResponse(source, ERR_UNKNOWNCOMMAND,
			"Unknown subcommand", subcommand));
}

/**
 *	NICK sets or changes the user's nickname. It's reference and unique identifier.
**/
void	Server::nick(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client				*source = commandParams.source;
	const std::string	&nickname = commandParams.arguments[0];

	if (nickname.length() > MAX_NICKNAME_LEN || nickname[0] == '#')
		errCommand(source, ERR_ERRONEUSNICKNAME, nickname, "Erroneous Nickname");

	{
		const Client	*concurrentNicknameHolder = getClient(nickname);
		if (concurrentNicknameHolder)
			errCommand(source, ERR_NICKNAMEINUSE, nickname, "Nickname is already in use");
	}

	source->setNickname(nickname);
	source->setServerPermissions(IDENTIFIED);

	source->receiveMessage(getServerResponse(source, RPL_WELCOME, "",
		"You are now known as " + nickname));
}

void	Server::user(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE)
		|| commandParams.arguments.size() < 3)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 3)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client	*source = commandParams.source;

	source->setUsername(commandParams.arguments[0]);
	source->setRealname(commandParams.message);

	source->receiveMessage(getServerResponse(source, RPL_WELCOME, "",
		"Welcome to our Internet Relay Chat Network !"));
}

void	Server::quit(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | POLLFD))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 0)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// clear necessary data if needed ? No response expected.

	commandParams.pollFd->revents |= POLLHUP;
}

/**
 *	JOIN handles connection tentatives to a Channel and Channel creation.
**/
void	Server::join(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client				*source = commandParams.source;
	const std::string	&channelName = commandParams.arguments[0];

	// Implement password management also here.

	if (channelName[0] != '#')
		errCommand(source, ERR_NOSUCHCHANNEL, channelName, "No such channel");

	Channel	*targetChannel = getChannel(channelName);

	if (targetChannel)
	{
		if (targetChannel->isFull())
			errCommand(source, ERR_CHANNELISFULL, channelName, "Channel is full");
		else if (targetChannel == source->getActiveChannel())
			errCommand(source, ERR_USERONCHANNEL, channelName, "Is already on channel");
		else
			targetChannel->addUser(source, targetChannel->getUserPerms());
	}
	else
	{
		targetChannel = new Channel(channelName, source);
		_channels[channelName] = targetChannel;
	}

	source->setActiveChannel(targetChannel);
	source->addToJoinedChannels(targetChannel);

	source->receiveMessage(getCommandResponse(source, "JOIN", targetChannel, ""));
	// Add additionnal channel info here.

	// TEMP
	//source->receiveMessage(":" + source->getNickname() + " JOIN " + channelName);
	//serverResponse(source, RPL_TOPIC, channelName, channel->getTopic());
	//serverResponse(source, RPL_TOPIC, channelName, channel->getTopic());
	//serverResponse(source, RPL_NAMREPLY, "= " + channelName, "usr1 user2 user3"); // get users list
	//serverResponse(source, RPL_ENDOFNAMES, channelName, "End of /NAMES list");
}

void	Server::whois(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	const Client	*source = commandParams.source;
	const Client	*targetUser = getClient(commandParams.arguments[0]);

	if (!targetUser)
		errCommand(commandParams.source, ERR_NOSUCHNICK, "", "No such nick/channel");
	source->receiveMessage(getServerResponse(commandParams.source, RPL_WHOREPLY, targetUser->getNickname() + " ~" + targetUser->getUsername() + " *", targetUser->getRealname()));
	// "<client> <nick> <username> <host> * :<realname>" --> should be adjusted to user without username or realname
	source->receiveMessage(getServerResponse(commandParams.source, RPL_ENDOFWHOIS, commandParams.arguments[0], "End of /WHOIS list"));
}

/**
 *	PRIVMSG handles communication between users and users/channels.
 *	It is used by default and targets the user's active channel when no command is given.
**/
void	Server::privmsg(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	const Client		*source = commandParams.source;
	std::string			targetName = commandParams.arguments[0];
	Channel				*targetChannel = NULL;
	Client				*targetClient = NULL;

	{
		const size_t	pos = targetName.find(":");

		if (pos != std::string::npos)
		{
			if (targetName.substr(pos + 1) != _serverSockets.getHostname())
				errCommand(source, ERR_NOPERMFORHOST, targetName, "Invalid hostname");
			targetName = targetName.substr(0, pos);
		}
	}

	if (targetName[0] == '#')
	{
		targetChannel = getChannel(targetName);
		if (!targetChannel)
			errCommand(source, ERR_NOSUCHCHANNEL, targetName, "No such channel");
	}
	else
	{
		targetClient = getClient(targetName);
		if (!targetClient)
			errCommand(source, ERR_NOSUCHNICK, targetName, "No such user");
	}

	const std::string	delimiter = DELIMITER;
	std::string			buffer = commandParams.message;
	std::string			message;
	size_t				pos;

	removeLeadingWhitespaces(buffer, delimiter);

	while (!buffer.empty())
	{
		if (buffer.length() >= MSG_BUFFER_SIZE)
		{
			pos = MSG_BUFFER_SIZE;
			pos = findLastChar(buffer, pos);
			message = buffer.substr(0, pos);
			buffer.erase(0, pos);
		}
		else
		{
			message = buffer;
			buffer.clear();
		}

		if (targetChannel)
			source->broadcastMessageToChannel(targetChannel,
				getCommandResponse(source, "PRIVMSG", targetChannel, message));
		else if (targetClient)
			targetClient->receiveMessage(getCommandResponse(source, "PRIVMSG",
				targetClient, message));

		removeLeadingWhitespaces(buffer, delimiter);
	}
}

void	Server::notice(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS | MESSAGE))
		errCommand(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Sends a server notice to a client, channel or everywhere.
	std::cout << "NOTICE command executed." << std::endl;
}

/**
 *	KICK removes forcefully a user from a Channel, affecting also it's privileges.
**/
void	Server::kick(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client			*source = commandParams.source;
	Channel			*targetChannel = NULL;
	Channel::User	*sourceUser = NULL;
	Channel::User	*targetUser = NULL;

	if (commandParams.arguments.size() == 1)
	{
		const std::string	&nickname = commandParams.arguments[0];

		if (nickname[0] == '#' || nickname.length() > MAX_NICKNAME_LEN)
			errCommand(source, ERR_ERRONEUSNICKNAME, nickname, "Erroneous Nickname");

		targetChannel = source->getActiveChannel();
		if (!targetChannel)
			errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");

		targetUser = targetChannel->getUser(nickname);
		if (!targetUser)
			errCommand(source, ERR_USERNOTINCHANNEL, targetChannel->getName(),
				"User not in that channel");
	}
	else if (commandParams.arguments.size() == 2)
	{
		const std::string	&channelName = commandParams.arguments[0];
		const std::string	&nickname = commandParams.arguments[1];

		if (channelName[0] != '#')
			errCommand(source, ERR_NOSUCHCHANNEL, channelName, "No such channel");
		else if (nickname[0] == '#' || nickname.length() > MAX_NICKNAME_LEN)
			errCommand(source, ERR_ERRONEUSNICKNAME, nickname, "Erroneous Nickname");

		targetChannel = getChannel(channelName);
		if (!targetChannel)
			errCommand(source, ERR_NOSUCHCHANNEL, channelName, "No such channel");

		targetUser = targetChannel->getUser(nickname);
		if (!targetUser)
			errCommand(source, ERR_USERNOTINCHANNEL, channelName, "User not in that channel");
	}

	sourceUser = targetChannel->getUser(source->getNickname());

	if (!sourceUser || areBitsNotSet(sourceUser->permissionsMask, Channel::KICK))
		errCommand(commandParams.source, ERR_CHANOPRIVSNEEDED, targetChannel->getName(),
			"Not enough privileges");

	targetChannel->removeUser(targetUser->client);

	targetUser->client->receiveMessage(getCommandResponse(source, "KICK",
		targetUser->client, commandParams.message));
}

void	Server::mode(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// ???
	std::cout << "MODE command executed." << std::endl;
}

void	Server::topic(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (areBitsSet(commandParams.mask, ARGUMENTS) && commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client		*source = commandParams.source;
	Channel		*targetChannel = NULL;

	if (commandParams.arguments.size() == 1)
	{
		const std::string	&targetName = commandParams.arguments[0];

		if (targetName[0] != '#')
			errCommand(source, ERR_NOSUCHCHANNEL, targetName, "No such channel");
		targetChannel = getChannel(targetName);
		if (!targetChannel)
			errCommand(source, ERR_NOSUCHCHANNEL, targetName, "No such channel");
	}
	else
	{
		targetChannel = source->getActiveChannel();
		if (!targetChannel)
			errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");
	}

	{
		const Channel::User *sourceUser = targetChannel->getUser(source->getNickname());
		if (!sourceUser)
			errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");
		else if (areBitsSet(commandParams.mask, MESSAGE)
			&& areBitsNotSet(sourceUser->permissionsMask, Channel::TOPIC))
			errCommand(source, ERR_CHANOPRIVSNEEDED, targetChannel->getName(),
				"Not enough privileges");
	}

	if (areBitsSet(commandParams.mask, MESSAGE))
		targetChannel->setTopic(commandParams.message);

	const std::string &topic = targetChannel->getTopic();

	if (topic.empty())
		source->receiveMessage(getServerResponse(source, RPL_NOTOPIC,
			targetChannel->getName(), "No topic is set"));
	else
		source->receiveMessage(getServerResponse(source, RPL_TOPIC,
			targetChannel->getName(), topic));
}

void	Server::invite(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS)
		|| commandParams.arguments.size() < 2)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Invites a client to a channel.
	// My current commandParams aren't adapted to this.
	std::cout << "INVITE command executed." << std::endl;
}

void	Server::who(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	const Client	*source = commandParams.source;
	const std::string	&target = commandParams.arguments[0];

	if (target[0] != '#')
	{
		Client	*targetUser = getClient(target);
		if (targetUser)
			source->receiveMessage(getServerResponse(commandParams.source, RPL_WHOREPLY, targetUser->getUsername() + " " + targetUser->getNickname(), targetUser->getRealname()));
	}
	// else
	// {
	// 	Channel		*targetChannel = getChannel(target);
	// 	if (targetChannel)
	// 	{

	// 	}

	// }
	source->receiveMessage(getServerResponse(commandParams.source, RPL_ENDOFWHO, target, "End of /WHO list"));

	// if arg[0][0] "#" -> list of channel members
	// else -> single user
	// "<client> <channel> <username> <host> <server> <nick> <flags> :<hopcount> <realname>"
	//  List the users in a channel (names, real names, server info, status, ...)
}

void	Server::names(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Lists users of a channel (nicknames and status)
	std::cout << "NAMES command executed." << std::endl;
}

void	Server::part(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	// Leaves a channel. A user can be member of multiple channels at the same time
	// for persistence.
	std::cout << "PART command executed." << std::endl;
}

void	Server::pass(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client	*source = commandParams.source;

	if (areBitsSet(source->getServerPermissions(), VERIFIED))
		errCommand(source, ERR_ALREADYREGISTERED, "",
			"You are already registered");
	else if (source->getConnectionRetries() >= MAX_CONNECTION_RETRIES)
	{
		commandParams.pollFd->revents |= POLLHUP;
		errCommand(source, ERR_PASSWDMISMATCH, "",
			"Acces denied. Too many password attempts.");
	}
	else if (_password != commandParams.arguments[0])
	{
		source->incrementConnectionRetries();
		errCommand(source, ERR_PASSWDMISMATCH, "",
			"Access denied. Password incorrect.");

	}
	else
		source->setServerPermissions(VERIFIED);
}

/* ************************************************************************** */
/* *                            Server utilities                            * */
/* ************************************************************************** */

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
		errCommand(client, ERR_PASSWDMISMATCH, "", "You have not verified");
		return (true);
	}
	else if (areBitsSet(mask, IDENTIFIED)
		&& areBitsNotSet(client->getServerPermissions(), IDENTIFIED))
	{
		errCommand(client, ERR_NOTREGISTERED, "", "You have not registered");
		return (true);
	}
	return (false);
}

void	Server::errCommand(const Client *client, const std::string &code,
	const std::string &parameter, const std::string &trailing)
{
	std::string	response;

	response = getServerResponse(client, code, parameter, trailing);
	client->receiveMessage(response);
	throw	std::runtime_error(response);
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

Client	*Server::getClient(const std::string &nickname)
{
	ClientsIterator	it = _clients.begin();

	while (it != _clients.end() && (*it)->getNickname() != nickname)
		it++;

	if (it == _clients.end())
		return (NULL);
	return (*it);
}

Channel	*Server::getChannel(const std::string &name)
{
	ChannelsIterator	it = _channels.find(name);

	if (it == _channels.end())
		return (NULL);
	return (it->second);

}

const std::string
Server::getServerResponse(const Client *client, const std::string &code,
	const std::string &parameters, const std::string &trailing) const
{
	std::string			response;
	const std::string	targetNickname = client->getNickname();

	response = ":" + _serverSockets.getHostname();
	response += " " + code;

	if (!targetNickname.empty())
		response += " " + targetNickname;
	else
		response += " *";

	if (!parameters.empty())
		response += " " + parameters;

	if (!trailing.empty())
		response += " :" + trailing;

	response += DELIMITER;

	return (response);
}

const std::string
Server::getCommandResponse(const Client *source, const std::string &command,
	const Client *target, const std::string &trailing) const
{
	std::string			response;

	response = ":" + source->getNickname();
	response += " " + command;
	response += " " + target->getNickname();

	if (!trailing.empty())
		response += " :" + trailing;

	response += DELIMITER;

	return (response);
}

const std::string
Server::getCommandResponse(const Client *source, const std::string &command,
	const Channel *target, const std::string &trailing) const
{
	std::string			response;

	response = ":" + source->getNickname();
	response += " " + command;
	response += " " + target->getName();

	if (!trailing.empty())
		response += " :" + trailing;

	response += DELIMITER;

	return (response);
}

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
	commandParameters.source = NULL;
	commandParameters.pollFd = NULL;

	if (source)
	{
		setBits(commandParameters.mask, SOURCE);
		commandParameters.source = source;
	}

	if (pollFd)
	{
		setBits(commandParameters.mask, POLLFD);
		commandParameters.pollFd = pollFd;
	}

	if (arguments.size() > 0)
	{
		setBits(commandParameters.mask, ARGUMENTS);
		commandParameters.arguments = arguments;
	}

	if (!message.empty())
	{
		setBits(commandParameters.mask, MESSAGE);
		commandParameters.message = message.substr(1);
	}

	return (commandParameters);
}

