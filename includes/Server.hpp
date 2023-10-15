/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/15 12:36:47 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ServerSocket.hpp"
#include "Client.hpp"

#include <vector>

// MACROS

class	Client;

class	Server
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		Server(const ASocket::t_soconfig &config);

		Server(const Server &other);
		Server	&operator=(const Server &other);

		virtual ~Server(void);

		/* Member functions */

		// Getter
		ServerSocket	&getSocket(void);

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
		ServerSocket	_socket;

		Clients			_clients;
		PollFds			_pollFds;

		/* Constructors & Destructors */
		Server(void);

		/* Member functions */
		void				eventLoop(void);

		void				handleClientConnections(struct pollfd &pollFd);
		bool				handleClientDataTransfers(Client *client, struct pollfd &pollFd);
		void				handleClientDisconnections(struct pollfd &pollFd, size_t &index);

};
