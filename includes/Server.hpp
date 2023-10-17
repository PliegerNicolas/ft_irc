/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/17 19:39:46 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ServerSockets.hpp"
#include "Client.hpp"

// MACROS

#define MSG_BUFFER_SIZE 256

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
		void				eventLoop(void);
};
