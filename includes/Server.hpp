/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mfaucheu <mfaucheu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/24 14:28:06 by nplieger         ###   ########.fr       */
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

#define DELIMITER "\n"			//"\r\n" for real IRC servers
#define MSG_BUFFER_SIZE 512
#define MAX_CONNECTION_RETRIES 3

class	Client;
class	Channel;

class	Server
{
	public:
		/* Attributs */

		typedef enum ServerPermissions
		{
			VERIFIED = (1 << 0),
			IDENTIFIED = (1 << 1)
		}	t_serverPermissions;

		/* Constructors & Destructors */
		Server(const ServerSockets::t_serverconfig &serverConfig, const std::string &password);

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

		typedef enum setCommandParameters
		{
			SOURCE = (1 << 0),
			ARGUMENTS = (1 << 1),
			MESSAGE = (1 << 2)
		}	t_setCommandParams;

		typedef struct CommandParameters
		{
			int							mask;
			Client						*source;
			std::vector<std::string>	arguments;
			std::string					message;
		}	t_commandParams;

		static t_commandParams	buildCommandParams(Client *source,
			std::vector<std::string> &arguments, std::string &message);

		typedef void (Server::*CommandFunction)(const t_commandParams &params);

		typedef std::vector<struct pollfd>				PollFds;
		typedef std::vector<Client*>					Clients;
		typedef std::map<std::string, Channel*>			Channels;
		typedef std::map<std::string, CommandFunction>	Commands;

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

		std::string		_password;

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
		void			setCommands(void);
		void			executeCommand(Client *client, std::string &clientBuffer,
							const std::string &delimiter);
		t_commandParams	parseCommand(Client *client, std::string &clientBuffer,
							const std::string &delimiter);

		void			cap(const t_commandParams &commandParams);
		void			nick(const t_commandParams &commandParams);
		void			user(const t_commandParams &commandParams);
		void			quit(const t_commandParams &commandParams);
		void			join(const t_commandParams &commandParams);
		void			whois(const t_commandParams &commandParams);
		void			privmsg(const t_commandParams &commandParams);
		void			notice(const t_commandParams &commandParams);
		void			kick(const t_commandParams &commandParams);
		void			mode(const t_commandParams &commandParams);
		void			topic(const t_commandParams &commandParams);
		void			invite(const t_commandParams &commandParams);
		void			who(const t_commandParams &commandParams);
		void			names(const t_commandParams &commandParams);
		void			part(const t_commandParams &commandParams);
		void			pass(const t_commandParams &commandParams);

		void			putMessage(Client *client, const std::string &delimiter, size_t &pos);
		bool			isCommand(const std::string &clientBuffer);
		bool			verifyServerPermissions(const Client *client, const int &mask);
};
