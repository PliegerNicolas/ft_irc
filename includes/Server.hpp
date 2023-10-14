/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 18:10:06 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ServerSocket.hpp"
#include "Client.hpp"

#include <deque>

// MACROS

class	Client;

class	Server
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		Server(const int &domain, const int &service, const int &protocol,
			const std::string &interface, const int &port);

		Server(const Server &other);
		Server	&operator=(const Server &other);

		virtual ~Server(void);

		/* Member functions */

		// Getter
		const ServerSocket	&getSocket(void) const;

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		ServerSocket				_socket;
		std::deque<struct pollfd>	_pollFds;

		std::deque<Client>			_clients;

		/* Constructors & Destructors */
		Server(void);

		/* Member functions */
		void				eventLoop(void);
};
