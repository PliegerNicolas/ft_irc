/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/06 14:49:10 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// INCLUDES

#include "socket/ServerSockets.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include "signals/signals.hpp"
#include "utils/utils.hpp"
#include "ircProtocolCodes.hpp"

#include <sstream>
#include <fstream>
#include <vector>
#include <map>

// MACROS

#define CLIENT_CONNECTED 1
#define CLIENT_DISCONNECTED 0

#define DELIMITER "\n" //"\r\n" for real IRC servers
#define MSG_BUFFER_SIZE 512
#define MAX_CONNECTION_RETRIES 3

#define MAX_USERNAME_LEN 18
#define MAX_NICKNAME_LEN 9
#define MAX_TOPIC_LEN 306
#define MAX_CHANNELNAME_LEN 50

#define MOTD_PATH "./config/MOTD.config"

#define SERVER_VERSION "ircserv-1.0.0 (alpha)"

class	Client;
class	Channel;

class	Server
{
	public:
		/* Typedefs */
		typedef enum ServerPermissions
		{
			VERIFIED = (1 << 0),
			IDENTIFIED = (1 << 1)
		}	t_serverPermissions;

		typedef enum setCommandParameters
		{
			SOURCE = (1 << 0),
			POLLFD = (1 << 1),
			ARGUMENTS = (1 << 2),
			MESSAGE = (1 << 3)
		}	t_setCommandParams;

		typedef struct CommandParameters
		{
			int							mask;
			Client						*source;
			struct pollfd				*pollFd;
			std::vector<std::string>	arguments;
			std::string					message;
		}	t_commandParams;

		static t_commandParams	buildCommandParams(Client *source, struct pollfd *pollFd,
			std::vector<std::string> &arguments, std::string &message);

		typedef void (Server::*CommandFunction)(const t_commandParams &params);
		typedef std::vector<struct pollfd>				PollFds;
		typedef std::map<std::string, CommandFunction>	Commands;
		typedef std::vector<Client*>					Clients;
		typedef std::map<std::string, Channel*>			Channels;
		typedef std::vector<std::string>				Arguments;

		typedef PollFds::iterator						PollFdsIterator;
		typedef Commands::iterator						CommandsIterator;
		typedef Clients::iterator						ClientsIterator;
		typedef Channels::iterator						ChannelsIterator;
		typedef Arguments::const_iterator				ArgumentsIterator;

		/* Attributs */

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
		/* Attributs */
		ServerSockets	_serverSockets;

		Commands		_commands;
		PollFds			_pollFds;
		Clients			_clients;
		Channels		_channels;

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
		void			executeCommand(Client *client, struct pollfd *pollFd,
							std::string &clientBuffer, const std::string &delimiter);
		t_commandParams	parseCommand(Client *client, struct pollfd *pollFd,
							std::string &clientBuffer, const std::string &delimiter);

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
		void			uninvite(const t_commandParams &commandParams);
		void			who(const t_commandParams &commandParams);
		void			names(const t_commandParams &commandParams);
		void			list(const t_commandParams &commandParams);
		void			motd(const t_commandParams &commandParams);
		void			part(const t_commandParams &commandParams);
		void			pass(const t_commandParams &commandParams);

		// Command utilities
		bool			isCommand(const std::string &clientBuffer);
		bool			verifyServerPermissions(const Client *client, const int &mask);
		void			errCommand(const Client *client, const std::string &code,
							const std::string &parameter, const std::string &trailing);

		ArgumentsIterator	parseMode(const t_commandParams commandParams,
								Channel::User *&targetUser, Channel *&targetChannel,
								std::string &modes);

		// Getters
		Client				*getClient(const std::string &nickname);
		Channel				*getChannel(const std::string &name);

		const std::string	getServerResponse(const Client *client, const std::string &code,
							const std::string &parameters, const std::string &trailing) const;
		const std::string	getCommandResponse(const Client *source, const std::string &command,
								const std::string &arguments, const std::string &trailing) const;

		// Setters
};
