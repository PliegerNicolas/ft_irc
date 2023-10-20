/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/20 15:07:45 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ServerSockets.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include "signals/signals.hpp"

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
		const struct pollfd	generatePollFd(const ASocket::t_socket &serverSocket);
		void				deleteClients(void);
		void				deleteChannels(void);

		// Getter

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Typedefs */
		typedef std::vector<struct pollfd>	PollFds;
		typedef std::vector<Client*>		Clients;
		typedef std::vector<Channel*>		Channels;

		typedef PollFds::iterator			PollFdsIterator;
		typedef Clients::iterator			ClientsIterator;
		typedef Channels::iterator			ChannelsIterator;

		/* Attributs */
		ServerSockets	_serverSockets;

		PollFds			_pollFds;
		Clients			_clients;
		Channels		_channels;

		/* Constructors & Destructors */
		Server(void);

		/* Member functions */
		void	eventLoop(void);

		void	handleServerPollFds(const ServerSockets::Sockets &serverSockets, size_t &i);
		void	handleClientsPollFds(const ServerSockets::Sockets &serverSockets, size_t &i);

		void	handleClientConnections(const ASocket::t_socket &serverSocket);
		bool	handleClientDataReception(Client *client, struct pollfd &pollFd);
		void	handleClientDisconnections(const ServerSockets::Sockets &serverSockets, size_t &i);

		// Utilites
		void	removeLeadingWhitespaces(std::string &str);
		size_t	findLastWordEnd(const std::string &str, const size_t &strLen);
};
