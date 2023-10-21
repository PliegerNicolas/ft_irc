/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/22 01:41:45 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ServerSockets.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include "signals/signals.hpp"
#include "utils/Utils.hpp"

#include <map>

// MACROS

#define CLIENT_CONNECTED 1
#define CLIENT_DISCONNECTED 0

#define DELIMITER "\n"
#define MSG_BUFFER_SIZE 512

class	Client;
class	Channel;

class	Server
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		Server(const ServerSockets::t_serverconfig &serverConfig);

		Server(const Server &other);
		Server	&operator=(const Server &other);

		virtual ~Server(void);

		/* Member functions */
		void					deleteClients(void);
		void					deleteChannels(void);
		const struct pollfd		generatePollFd(const ASocket::t_socket &serverSocket);

		// Getter

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Typedefs */
		typedef struct CommandParameters
		{
			Client		*who;
			const void	*target;
			const char	*message;
		}	t_commandParams;

		static const t_commandParams	buildCommandParams(Client *who, const void *target,
			const char *message);

		typedef void (Server::*commandFunction)(const t_commandParams &params);

		typedef std::vector<struct pollfd>				PollFds;
		typedef std::vector<Client*>					Clients;
		typedef std::map<std::string, Channel*>			Channels;
		typedef std::map<std::string, commandFunction>	Commands;

		typedef PollFds::iterator						PollFdsIterator;
		typedef Clients::iterator						ClientsIterator;
		typedef Channels::iterator						ChannelsIterator;
		typedef Commands::iterator						CommandsIterator;

		/* Attributs */
		ServerSockets	_serverSockets;

		PollFds			_pollFds;
		Clients			_clients;
		Channels		_channels;
		Commands		_commands;

		/* Constructors & Destructors */
		Server(void);

		/* Member functions */

		// Event loop
		void	eventLoop(void);
		void	handleServerPollFds(const ServerSockets::Sockets &serverSockets, size_t &i);
		void	handleClientsPollFds(const ServerSockets::Sockets &serverSockets, size_t &i);

		// Server interactions
		void	handleClientConnections(const ASocket::t_socket &serverSocket);
		bool	handleClientDataReception(Client *client, struct pollfd &pollFd);
		void	handleClientDisconnections(const ServerSockets::Sockets &serverSockets, size_t &i);

		// Commands
		void	setCommands(void);
		void	executeCommand(Client *client, std::string &clientBuffer);
		void	nick(const t_commandParams &commandParams);
		void	quit(const t_commandParams &commandParams);
		void	join(const t_commandParams &commandParams);
		void	whois(const t_commandParams &commandParams);
		void	privmsg(const t_commandParams &commandParams);
		void	notice(const t_commandParams &commandParams);
		void	kick(const t_commandParams &commandParams);
		void	mode(const t_commandParams &commandParams);
		void	topic(const t_commandParams &commandParams);
		void	invite(const t_commandParams &commandParams);
		void	who(const t_commandParams &commandParams);
		void	names(const t_commandParams &commandParams);
		void	part(const t_commandParams &commandParams);

		void	putMessage(std::string &clientBuffer, const std::string &delimiter, size_t &pos);
};
