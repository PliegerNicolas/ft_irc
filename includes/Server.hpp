/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/19 11:09:29 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ServerSockets.hpp"
#include "Client.hpp"

// MACROS

#define CLIENT_CONNECTED 1
#define CLIENT_DISCONNECTED 0

#define DELIMITER "\n"
#define MSG_BUFFER_SIZE 512

class	Client;

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

		// Getter

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Typedefs */
		typedef std::vector<Client*>		Clients;
		typedef std::vector<struct pollfd>	PollFds;

		typedef Clients::iterator			ClientsIterator;
		typedef PollFds::iterator			PollFdsIterator;

		/* Attributs */
		ServerSockets	_serverSockets;

		Clients			_clients;
		PollFds			_pollFds;

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
		void	deleteClients(Server::Clients &clients);
};
