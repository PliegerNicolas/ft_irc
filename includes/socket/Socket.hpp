/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 13:23:07 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/13 19:11:52 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "colors.hpp"
#include "debug.hpp"

#include <iostream>
#include <sstream>

#include <unistd.h>
#include <cstring>

#include <sys/socket.h>
#include <sys/poll.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

// MACROS

#define MAX_PORT 65535
#define MIN_PORT 0

class	Socket
{
	public:
		/* Typedefs */
		typedef struct SocketConfig
		{
			int			domain;		// communication domain (IPv4, IPv6, AF_UNIX, ...)
			int			service;	// communication semantics (stream, dgream, raw, ...)
			int			protocol;	// transmission protocol (TCP, UDP, HTTP, ...)
			std::string	interface;	// IP address
			int			port;		// Port
		} t_soconfig;

		static const t_soconfig	buildSocketConfig(const int &domain, const int &service,
			const int &protocol, const std::string &interface, const int &port);

		/* Attributs */

		/* Constructors & Destructors */
		Socket(void);

		Socket(const Socket &other);
		Socket	&operator=(const Socket &other);

		virtual ~Socket(void);

		/* Member functions */

		// Getter
		const struct pollfd		&getPoll(void) const;
		const struct sockaddr	*getAddress(void) const;

		const int				&getSocketFd(void) const;
		std::string				getIP(void) const;
		uint16_t				getPort(void) const;

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */
		struct sockaddr_in	_address;
		struct pollfd		_poll;

		/* Member functions */
		void					handleSocketErrors(const int &statusCode);

	private:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

};
