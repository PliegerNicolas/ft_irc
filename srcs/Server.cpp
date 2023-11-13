/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/13 12:28:47 by hania            ###   ########.fr       */
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


		if (_clients.size() <= 0)
			client->setClientModesMask(Client::OPERATOR);

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
	Client			*client = *clientIt;
	Channels		&joinedChannels = client->getJoinedChannels();

	client->setActiveChannel(NULL);

	for (ChannelsIterator it = joinedChannels.begin(); it != joinedChannels.end(); it++)
	{
		Channel	*targetChannel = it->second;

		client->broadcastMessageToChannel(targetChannel, getCommandResponse(client, "QUIT",
			targetChannel->getName(), "Leaving the IRC server"));

		targetChannel->removeUser(client);
		targetChannel->removeInvitation(client);

		if (targetChannel->getUsers().size() > 0)
		{
			Channel::User	*newOwner = targetChannel->findFirstHighestPrivilege();
			setBits(newOwner->modesMask, Channel::OWNER);
		}
		else
		{
			_channels.erase(targetChannel->getName());
			delete targetChannel;
		}
	}

	_clients.erase(clientIt);
	delete client;

	_pollFds.erase(_pollFds.begin() + i--);
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
	_commands["UNINVITE"] = &Server::uninvite;
	_commands["WHOIS"] = &Server::whois;
	_commands["PRIVMSG"] = &Server::privmsg;
	_commands["MODE"] = &Server::mode;
	_commands["KICK"] = &Server::kick;
	_commands["NOTICE"] = &Server::notice;
	_commands["LIST"] = &Server::list;
	_commands["WHO"] = &Server::who;
	_commands["NAMES"] = &Server::names;
	_commands["TOPIC"] = &Server::topic;
	_commands["MOTD"] = &Server::motd;
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

void	Server::cap(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS) || commandParams.arguments.size() > 2)
		errCommand(commandParams.source, ERR_INVALIDCAPCMD, "", "Invalid CAP command");

	const Client	*source = commandParams.source;
	std::string		subcommand = commandParams.arguments[0];

	if (subcommand == "LS" || subcommand == "REQ" || subcommand == "ACK" || subcommand == "NAK")
		source->receiveMessage(getServerResponse(source, ERR_INVALIDCAPCMD,
			"CAP " + subcommand, "Capability negotiation is not supported"));
	else if (subcommand != "END")
		source->receiveMessage(getServerResponse(source, ERR_INVALIDCAPCMD,
			subcommand, "Unknown subcommand"));
}

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

	if (!Client::isValidNickname(nickname))
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
	motd(buildCommandParams(commandParams.source, commandParams.pollFd, Arguments(), ""));
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

	Client				*source = commandParams.source;
	const std::string	&username = commandParams.arguments[0];
	const std::string	&hostname = commandParams.arguments[1];
	const std::string	&servername = commandParams.arguments[2];
	const std::string	&realname = commandParams.message;

	source->setUsername(username);
	source->setHostname(hostname);
	source->setServername(servername);
	source->setRealname(realname);

	source->receiveMessage(getServerResponse(source, RPL_WELCOME, "",
		"Welcome to our Internet Relay Chat Network !"));
}

void	Server::quit(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE | POLLFD))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 0)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	commandParams.pollFd->revents |= POLLHUP;
}

void	Server::join(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client				*source = commandParams.source;
	const std::string	&channelName = commandParams.arguments[0];

	if (channelName[0] != '#')
		errCommand(source, ERR_NOSUCHCHANNEL, channelName, "No such channel");

	Channel	*targetChannel = getChannel(channelName);

	if (targetChannel)
	{
		if (areBitsSet(targetChannel->getChannelModesMask(), Channel::INVITE_ONLY)
			&& !targetChannel->isInvited(source))
			errCommand(source, ERR_INVITEONLYCHAN, channelName, "You are not invited (+i)");
		else if (areBitsSet(targetChannel->getChannelModesMask(), Channel::USER_LIMIT)
			&& targetChannel->isFull())
			errCommand(source, ERR_CHANNELISFULL, channelName, "Cannot join channel (+l)");
		else if (areBitsSet(targetChannel->getChannelModesMask(), Channel::KEY_PASS))
		{
			if (commandParams.arguments.size() < 2)
				errCommand(source, ERR_BADCHANNELKEY, channelName, "Cannot join channel (+k)");

			const std::string	&password = commandParams.arguments[1];

			if (password != targetChannel->getPassword())
				errCommand(source, ERR_BADCHANNELKEY, channelName, "Cannot join channel (+k)");
		}

		if (targetChannel == source->getActiveChannel())
			errCommand(source, ERR_USERONCHANNEL, channelName, "Is already on channel");

		if (targetChannel->isClientRegistered(source))
			source->setActiveChannel(targetChannel);
		else
			source->joinChannel(targetChannel);
	}
	else
	{
		targetChannel = new Channel(channelName);
		_channels[channelName] = targetChannel;
		source->joinChannel(targetChannel);
		targetChannel->setUserModesMask(targetChannel->getUser(source->getNickname()),
			Channel::defaultOwnerPerms());
	}

	std::string	commandResponse = getCommandResponse(source, "JOIN", targetChannel->getName(), "");

	source->receiveMessage(commandResponse);
	source->broadcastMessageToChannel(targetChannel, commandResponse);
	who(commandParams);
	names(commandParams);
}

void	Server::whois(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NONICKNAMEGIVEN, "", "No nickname given");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	const Client		*source = commandParams.source;
	const std::string	&targetName = commandParams.arguments[0];

	Client				*targetClient = getClient(targetName);

	if (targetName[0] != '#' && targetClient)
	{
		{
			std::string	info = targetClient->getNickname();

			info += " " + targetClient->getPrefix() + targetClient->getUsername();

			if (!targetClient->getHostname().empty())
				info += " " + targetClient->getHostname();
			else
				info += " *";

			info += " *";

			source->receiveMessage(getServerResponse(source, RPL_WHOISUSER, info,
				targetClient->getRealname()));
		}
		{
			std::string	info = "is using modes " + targetClient->getClientModes();

			source->receiveMessage(getServerResponse(source, RPL_UMODEIS,
				targetClient->getNickname(), info));
		}

		if (areBitsSet(targetClient->getClientModesMask(), Client::INVISIBLE)
			&& targetClient->getActiveChannel())
		{
			std::string	info = targetClient->getActiveChannel()->getName();

			source->receiveMessage(getServerResponse(source, RPL_UMODEIS,
				targetClient->getNickname(), info));
		}
	}
	else
		errCommand(source, ERR_NOSUCHNICK, targetName, "No such user");

	source->receiveMessage(getServerResponse(source, RPL_ENDOFWHOIS, targetName,
		"End of /WHOIS list"));
}

void	Server::privmsg(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NORECIPIENT, "", "No recipient given (PRIVMSG)");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");
	else if (areBitsNotSet(commandParams.mask, MESSAGE) || commandParams.message.empty())
		errCommand(commandParams.source, ERR_NOTEXTTOSEND, "", "No text to send");
	const Client	*source = commandParams.source;
	std::string		targetName = commandParams.arguments[0];
	Channel			*targetChannel = NULL;
	Client			*targetClient = NULL;

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
			errCommand(source, ERR_NOSUCHNICK, targetName, "No such channel");
	}
	else
	{
		targetClient = getClient(targetName);
		if (!targetClient)
			errCommand(source, ERR_NOSUCHNICK, targetName, "No such user");
	}

	if (targetChannel && !targetChannel->canTalk(source))
		errCommand(source, ERR_CANNOTSENDTOCHAN, targetChannel->getName(),
			"You need voice (+v) (" + targetChannel->getName() + ")");

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
				getCommandResponse(source, "PRIVMSG", targetChannel->getName(), message));
		else if (targetClient)
			targetClient->receiveMessage(getCommandResponse(source, "PRIVMSG",
				targetClient->getNickname(), message));

		removeLeadingWhitespaces(buffer, delimiter);
	}
}

// TODO: Implement
// Sends a server notice to a client, channel or everywhere.

// 404     ERR_CANNOTSENDTOCHAN
                // // "<channel name> :Cannot send to channel
        // - Sent to a user who is either (a) not on a channel
        //   which is mode +n or (b) not a chanop (or mode +v) on
        //   a channel which has mode +m set and is trying to send
        //   a PRIVMSG message to that channel.

void	Server::notice(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NORECIPIENT, "", "No recipient given (NOTICE)");
	else if (commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");
	else if (areBitsNotSet(commandParams.mask, MESSAGE) || commandParams.message.empty())
		errCommand(commandParams.source, ERR_NOTEXTTOSEND, "", "No text to send");
	const Client	*source = commandParams.source;
	std::string		targetName = commandParams.arguments[0];
	Channel			*targetChannel = NULL;
	Client			*targetClient = NULL;

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
			errCommand(source, ERR_NOSUCHNICK, targetName, "No such channel");
	}
	else
	{
		targetClient = getClient(targetName);
		if (!targetClient)
			errCommand(source, ERR_NOSUCHNICK, targetName, "No such user");
	}

	if (targetChannel && !targetChannel->canTalk(source))
		errCommand(source, ERR_CANNOTSENDTOCHAN, targetChannel->getName(),
			"You need voice (+v) (" + targetChannel->getName() + ")");

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
				getCommandResponse(source, "NOTICE", targetChannel->getName(), message));
		else if (targetClient)
			targetClient->receiveMessage(getCommandResponse(source, "NOTICE",
				targetClient->getNickname(), message));

		removeLeadingWhitespaces(buffer, delimiter);
	}
}

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
	Channel::User	*targetUser = NULL;

	if (commandParams.arguments.size() == 1)
	{
		const std::string	&nickname = commandParams.arguments[0];

		if (!Client::isValidNickname(nickname))
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
		else if (!Client::isValidNickname(nickname))
			errCommand(source, ERR_ERRONEUSNICKNAME, nickname, "Erroneous Nickname");

		targetChannel = getChannel(channelName);
		if (!targetChannel)
			errCommand(source, ERR_NOSUCHCHANNEL, channelName, "No such channel");

		targetUser = targetChannel->getUser(nickname);
		if (!targetUser)
			errCommand(source, ERR_USERNOTINCHANNEL, channelName, "User not in that channel");
	}

	if (!targetChannel->canKick(source))
		errCommand(commandParams.source, ERR_CHANOPRIVSNEEDED, targetChannel->getName(),
			"Not enough privileges");

	std::string		commandResponse;
	Client			*targetClient = targetUser->client;

	targetUser->client->quitChannel(targetChannel);

	commandResponse = getCommandResponse(source, "KICK", targetChannel->getName()
						+ " " + targetClient->getNickname(), commandParams.message);

	source->receiveMessage(commandResponse);
	targetClient->receiveMessage(commandResponse);
}

void	Server::mode(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");

	Client				*source = commandParams.source;
	Client				*targetClient = NULL;
	Channel				*targetChannel = NULL;
	std::string			modes;
	char				sign = '\0';

	ArgumentsIterator	it = parseMode(commandParams, targetClient, targetChannel, modes);
	ArgumentsIterator	itEnd = commandParams.arguments.end();

	stripDuplicateChars(modes);

	if (!modes.empty())
	{
		sign = modes[0];
		modes = modes.substr(1);
	}

	if (targetChannel && !targetClient)
	{
		if (!modes.empty())
		{
			if (!targetChannel->canUpdateModes(source))
				errCommand(commandParams.source, ERR_CHANOPRIVSNEEDED,
					targetChannel->getName(), "Not enough privileges");

			Channel::User	*targetUser = NULL;
			int				modeStatus = MODE_UNCHANGED;

			if (std::distance(it, itEnd) > 0 && (targetUser = targetChannel->getUser(*it)))
				it++;

			if (targetUser)
			{
				if (sign == '+' && std::distance(it, itEnd) < validatePresenceInString(modes, ""))
					errCommand(source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");

				for (size_t i = 0; i < modes.length(); i++)
				{
					if (sign == '+')
						modeStatus = targetChannel->addUserMode(targetUser, modes[i], "");
					else if (sign == '-')
						modeStatus = targetChannel->removeUserMode(targetUser, modes[i]);

					if (modeStatus == MODE_CHANGED)
						source->receiveMessage(getCommandResponse(source, "MODE",
							targetChannel->getName() + " " + sign + modes[i]
							+ " " + targetUser->client->getNickname(), ""));
					else if (modeStatus == MODE_INVALID)
						errCommand(source, ERR_UNKNOWNMODE,
							targetChannel->getName() + " " + sign + modes[i], "Unknown mode char");
				}
			}
			else
			{
				if (sign == '+' && std::distance(it, itEnd) < validatePresenceInString(modes, "kl"))
					errCommand(source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");

				for (size_t i = 0; i < modes.length(); i++)
				{
					std::string	argument;

					if (sign == '+')
					{
						if (modes[i] == 'k' || modes[i] == 'l')
						{
							modeStatus = targetChannel->addChannelMode(modes[i], *it++);
							if (modes[i] == 'k')
								argument = " " + targetChannel->getPassword();
							else if (modes[i] == 'l')
								argument = " " + targetChannel->getUserLimit();
						}
						else
							modeStatus = targetChannel->addChannelMode(modes[i], "");
					}
					else if (sign == '-')
						modeStatus = targetChannel->removeChannelMode(modes[i]);

					if (modeStatus == MODE_CHANGED)
						source->receiveMessage(getCommandResponse(source, "MODE",
							targetChannel->getName() + " " + sign + modes[i] + argument, ""));
					else if (modeStatus == MODE_INVALID)
						errCommand(source, ERR_UNKNOWNMODE,
							targetChannel->getName() + " " + sign + modes[i], "Unknown mode char");
				}
			}
		}
		else
			source->receiveMessage(getServerResponse(source, RPL_CHANNELMODEIS,
				targetChannel->getName() + " " + targetChannel->getChannelModes(), ""));
	}
	else if (!targetChannel && targetClient)
	{
		if (!modes.empty())
		{
			if (areBitsNotSet(source->getClientModesMask(), Client::OPERATOR))
				errCommand(commandParams.source, ERR_NOPRIVILEGES,
					targetClient->getNickname(), "Not enough privileges");

			int	modeStatus = MODE_UNCHANGED;

			for (size_t i = 0; i < modes.length(); i++)
			{
				if (sign == '+')
					modeStatus = targetClient->addClientMode(modes[i], "");
				else if (sign == '-')
					modeStatus = targetClient->removeClientMode(modes[i]);

				if (modeStatus == MODE_CHANGED)
					source->receiveMessage(getCommandResponse(source, "MODE",
						targetClient->getNickname() + " " + sign + modes[i], ""));
				else if (modeStatus == MODE_INVALID)
					errCommand(source, ERR_UNKNOWNMODE,
						targetClient->getNickname() + " " + sign + modes[i], "Unknown mode char");
			}
		}
		else
			source->receiveMessage(getServerResponse(source, RPL_UMODEIS,
				targetClient->getNickname() + " " + targetClient->getClientModes(), ""));
	}
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

	if (commandParams.arguments.size() == 0)
	{
		targetChannel = source->getActiveChannel();
		if (!targetChannel)
			errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");
	}
	else if (commandParams.arguments.size() == 1)
	{
		const std::string	&targetName = commandParams.arguments[0];

		if (targetName[0] != '#')
			errCommand(source, ERR_NOSUCHCHANNEL, targetName, "No such channel");

		targetChannel = getChannel(targetName);
		if (!targetChannel)
			errCommand(source, ERR_NOSUCHCHANNEL, targetName, "No such channel");
	}

	if (areBitsSet(commandParams.mask, MESSAGE))
	{
		if (!targetChannel->canChangeTopic(source))
			errCommand(source, ERR_CHANOPRIVSNEEDED, targetChannel->getName(),
				"Not enough privileges");

		targetChannel->setTopic(commandParams.message);
	}

	const std::string &topic = targetChannel->getTopic();

	if (topic.empty())
		source->receiveMessage(getServerResponse(source, RPL_NOTOPIC,
			targetChannel->getName(), "No topic is set"));
	else
		source->receiveMessage(getServerResponse(source, RPL_TOPIC,
			targetChannel->getName(), topic));
}

// TODO: list invited users
// Stopped here

void	Server::invite(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client				*source = commandParams.source;
	Channel				*targetChannel = NULL;

	const std::string	&targetNickname = commandParams.arguments[0];

	if (!Client::isValidNickname(targetNickname))
		errCommand(source, ERR_NOSUCHNICK, targetNickname, "No such user");

	// List all invited users if without arguments in channel ?
	// RPL_INVITELIST and RPL_ENDOFINVITELIST

	if (commandParams.arguments.size() == 1)
	{
		targetChannel = source->getActiveChannel();
		if (!targetChannel)
			errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");
	}
	else if (commandParams.arguments.size() == 2)
	{
		const std::string	&targetChannelName = commandParams.arguments[1];

		targetChannel = getChannel(targetChannelName);

		if (targetChannelName[0] != '#' || !targetChannel)
			errCommand(source, ERR_NOSUCHCHANNEL, targetChannelName, "No such channel");
	}

	if (!targetChannel->canInvite(source))
		errCommand(commandParams.source, ERR_CHANOPRIVSNEEDED,
			targetChannel->getName(), "Not enough privileges");

	Channel::User	*targetUser = targetChannel->getUser(targetNickname);
	if (targetUser)
		errCommand(source, ERR_USERONCHANNEL, targetNickname, "User already on channel");

	Client			*targetClient = getClient(targetNickname);
	if (!targetClient)
		errCommand(source, ERR_NOSUCHNICK, targetNickname, "No such user");

	targetChannel->addInvitation(targetClient);

	source->receiveMessage(getServerResponse(source, RPL_INVITING,
		targetNickname + " " + targetChannel->getName(), ""));
	targetClient->receiveMessage(getCommandResponse(source, "INVITE",
		targetNickname, targetChannel->getName()));
}

// TODO: correct response.

void	Server::uninvite(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE | ARGUMENTS))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (commandParams.arguments.size() > 2)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client				*source = commandParams.source;
	Channel				*targetChannel = NULL;

	const std::string	&targetNickname = commandParams.arguments[0];

	if (!Client::isValidNickname(targetNickname))
		errCommand(source, ERR_NOSUCHNICK, targetNickname, "No such user");

	if (commandParams.arguments.size() == 1)
	{
		targetChannel = source->getActiveChannel();
		if (!targetChannel)
			errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");
	}
	else if (commandParams.arguments.size() == 2)
	{
		const std::string	&targetChannelName = commandParams.arguments[1];

		targetChannel = getChannel(targetChannelName);

		if (targetChannelName[0] != '#' || !targetChannel)
			errCommand(source, ERR_NOSUCHCHANNEL, targetChannelName, "No such channel");
	}

	if (!targetChannel->canInvite(source))
		errCommand(commandParams.source, ERR_CHANOPRIVSNEEDED,
			targetChannel->getName(), "Not enough privileges");

	Channel::User	*targetUser = targetChannel->getUser(targetNickname);
	if (targetUser)
		errCommand(source, ERR_USERONCHANNEL, targetNickname, "User already on channel");

	Client			*targetClient = getClient(targetNickname);
	if (!targetClient)
		errCommand(source, ERR_NOSUCHNICK, targetNickname, "No such user");
	targetChannel->removeInvitation(targetClient);

	source->receiveMessage(getServerResponse(source, RPL_INVITING,
		targetNickname + " " + targetChannel->getName(), ""));
	targetClient->receiveMessage(getCommandResponse(source, "UNINVITE",
		targetNickname, targetChannel->getName()));
}

//TODO: add filter mask ? /WHO <name> ["o"] for operator ?
// Stopped here

void	Server::who(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (areBitsNotSet(commandParams.mask, ARGUMENTS) && commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client			*source = commandParams.source;
	std::string		targetName;
	std::string		info;

	if (areBitsSet(commandParams.mask, ARGUMENTS) && commandParams.arguments[0][0] != '#')
	{
		targetName = commandParams.arguments[0];
		const Client		*targetClient = getClient(targetName);

		if (targetClient)
		{
			info = targetName;
			info += " " + targetClient->getHostname();
			info += " " + targetClient->getUsername();

			source->receiveMessage(getServerResponse(source, RPL_WHOREPLY, info,
				targetClient->getRealname()));
		}
	}
	else
	{
		Channel	*targetChannel = NULL;

		if (areBitsSet(commandParams.mask, ARGUMENTS))
		{
			targetName = commandParams.arguments[0];
			targetChannel = getChannel(targetName);
		}
		else
		{
			targetChannel = source->getActiveChannel();
			if (!targetChannel)
				errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");
			targetName = targetChannel->getName();
		}

		if (targetChannel)
		{
			const Channel::Users	&users = targetChannel->getUsers();

			for (Channel::UsersConstIterator it = users.begin(); it != users.end(); it++)
			{
				info = targetName;
				info += " " + it->client->getUsername();
				info += " " + it->client->getHostname();
				info += " " + it->client->getServername();
				info += " " + it->client->getNickname();
				// add info about role. Place holder is H.
				info += " Hs@";

				source->receiveMessage(getServerResponse(source, RPL_WHOREPLY, info,
					"0 " + it->client->getRealname()));
			}
		}
	}

	source->receiveMessage(getServerResponse(source, RPL_ENDOFWHO, targetName,
		"End of /WHO list"));
}

void	Server::names(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (areBitsNotSet(commandParams.mask, ARGUMENTS) && commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client			*source = commandParams.source;
	Channel			*targetChannel = NULL;
	std::string		targetName;
	std::string		info;

	if (areBitsSet(commandParams.mask, ARGUMENTS))
	{
		targetName = commandParams.arguments[0];

		if (targetName[0] != '#')
			errCommand(source, ERR_NOSUCHCHANNEL, targetName, "No such channel");

		targetChannel = getChannel(targetName);
	}
	else
	{
		targetChannel = source->getActiveChannel();
		targetName = targetChannel->getName();

		if (!targetChannel)
			errCommand(source, ERR_NOTONCHANNEL, "", "You are not on a channel");
	}

	if (targetChannel)
	{
		const Channel::Users	&users = targetChannel->getUsers();

		for (Channel::UsersConstIterator it = users.begin(); it != users.end(); it++)
		{
			// add info about role as prefix. No placeholder yet.
			if (it != users.begin())
				info += " " + it->client->getNickname();
			else
				info += it->client->getNickname();
		}

		source->receiveMessage(getServerResponse(source, RPL_NAMREPLY,
			"= " + targetName, info));
	}

	source->receiveMessage(getServerResponse(source, RPL_ENDOFNAMES, targetName,
		"End of /NAMES list"));
}

void	Server::list(const t_commandParams &commandParams) {

	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (!commandParams.arguments.empty())
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	const Client		*source = commandParams.source;
	std::string			info;

	if (!_channels.empty())
	{
		source->receiveMessage(getServerResponse(source, RPL_LISTSTART,
			"Channel", "Users	Name"));

		for (ChannelsIterator it = _channels.begin(); it != _channels.end(); it++)
		{
			const Channel		*targetChannel = it->second;
			std::stringstream	ss;

			ss << targetChannel->getName();
			ss << " " << targetChannel->getUsers().size();
			ss << " :";
			if (!targetChannel->getTopic().empty())
				ss << targetChannel->getTopic();

			source->receiveMessage(getServerResponse(source, RPL_LIST, ss.str(), ""));
		}
	}

	source->receiveMessage(getServerResponse(source, RPL_LISTEND, "", "End of /LIST"));
}

void	Server::motd(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (areBitsSet(commandParams.mask, ARGUMENTS | MESSAGE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	const Client	*source = commandParams.source;
	std::ifstream	motdFile(MOTD_PATH);
	std::string		line;

	if (!motdFile.is_open())
		errCommand(source, ERR_NOMOTD, "", strerror(errno));

	source->receiveMessage(getServerResponse(source, RPL_MOTDSTART,
		"", "- " + _serverSockets.getHostname() + " Message of the day -"));

	while (getline(motdFile, line))
		source->receiveMessage(getServerResponse(source, RPL_MOTD, "", line));

	source->receiveMessage(getServerResponse(source, RPL_ENDOFMOTD, "", "End of /MOTD command"));
}

void	Server::part(const t_commandParams &commandParams)
{
	if (verifyServerPermissions(commandParams.source, VERIFIED | IDENTIFIED))
		return ;
	else if (areBitsNotSet(commandParams.mask, SOURCE))
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Not enough parameters");
	else if (areBitsSet(commandParams.mask, ARGUMENTS)
		&& commandParams.arguments.size() > 1)
		errCommand(commandParams.source, ERR_NEEDMOREPARAMS, "", "Too many parameters");

	Client	*source = commandParams.source;
	Channel	*targetChannel = NULL;
	bool	isSourceChannelOwner = false;

	if (areBitsSet(commandParams.mask, ARGUMENTS))
	{
		const std::string	&channelName = commandParams.arguments[0];

		if (channelName[0] != '#')
			errCommand(source, ERR_NOSUCHCHANNEL, channelName, "No such channel");

		targetChannel = getChannel(channelName);
	}
	else
		targetChannel = source->getActiveChannel();

	if (!targetChannel)
		errCommand(source, ERR_NOTONCHANNEL, "", "You are not on that channel");

	{
		Channel::User	*sourceUser = targetChannel->getUser(source->getNickname());

		if (!sourceUser)
		errCommand(source, ERR_NOTONCHANNEL, targetChannel->getName(),
			"You are not on that channel");

		isSourceChannelOwner = areBitsSet(sourceUser->modesMask, Channel::OWNER);
	}

	source->quitChannel(targetChannel);

	if (isSourceChannelOwner && targetChannel->getUsers().size() > 0)
	{
		Channel::User	*newOwner = targetChannel->findFirstHighestPrivilege();
		setBits(newOwner->modesMask, Channel::OWNER);
	}
	else
	{
		delete targetChannel;
		targetChannel = NULL;
	}

	std::string	response = getCommandResponse(source, "PART",
		targetChannel->getName(), commandParams.message);

	if (targetChannel)
		source->broadcastMessageToChannel(targetChannel, response);
	source->receiveMessage(response);
}

void	Server::pass(const t_commandParams &commandParams)
{
	if (areBitsNotSet(commandParams.mask, SOURCE))
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
	throw std::runtime_error(response);
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
	const std::string &arguments, const std::string &trailing) const
{
	std::string			response;

	response = ":" + source->getNickname();
	response += " " + command;

	if (!arguments.empty())
		response += " " + arguments;

	if (!trailing.empty())
		response += " :" + trailing;

	response += DELIMITER;

	return (response);
}

Server::ArgumentsIterator	Server::parseMode(const t_commandParams &commandParams,
	Client *&targetClient, Channel *&targetChannel, std::string &modes)
{
	Client				*source = commandParams.source;
	ArgumentsIterator	it = commandParams.arguments.begin();

	if (areBitsSet(commandParams.mask, ARGUMENTS))
	{
		for (; modes.empty() && it != commandParams.arguments.end(); it++)
		{
			const std::string	&arg = *it;

			switch (arg[0])
			{
				case '+':
					if (!modes.empty())
						errCommand(source, ERR_UNKNOWNCOMMAND, arg, "Unknown command");
					modes = arg;
					break ;
				case '-':
					if (!modes.empty())
						errCommand(source, ERR_UNKNOWNCOMMAND, arg, "Unknown command");
					modes = arg;
					break ;
				case '#':
					if (targetChannel)
						errCommand(source, ERR_UNKNOWNCOMMAND, arg, "Unknown command");
					targetChannel = getChannel(arg);
					if (!targetChannel)
						errCommand(source, ERR_NOSUCHCHANNEL, arg, "No such channel");
					break ;
				default:
					if (!targetChannel)
					{
						targetClient = getClient(arg);
						if (!targetClient)
							errCommand(source, ERR_NOSUCHNICK, arg, "No such user");
					}
					else
						modes = "+" + arg;
					break ;
			}
		}
	}

	if (!targetClient && !targetChannel)
	{
		Channel	*activeChannel = source->getActiveChannel();
		if (activeChannel)
			targetChannel = activeChannel;
		else
			targetClient = source;
	}

	return (it);
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
	const Arguments &arguments, const std::string &message)
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
