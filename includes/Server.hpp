/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:48:29 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 11:56:05 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ServerSocket.hpp"
#include "Client.hpp"

#include <list>

// MACROS

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

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		ServerSocket				_socket;
		std::list<struct pollfd>	_pollFds;

		std::list<Client>			_clients;

		/* Constructors & Destructors */
		Server(void);

		/* Member functions */
};
