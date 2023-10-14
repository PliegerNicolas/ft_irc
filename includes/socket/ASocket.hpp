/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 13:23:07 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 18:05:14 by nicolas          ###   ########.fr       */
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
#include <cerrno>

#include <sys/socket.h>
#include <sys/poll.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

// MACROS

#define MAX_PORT 65535
#define MIN_PORT 0

class	ASocket
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

		typedef struct SocketOption
		{
			int	level;
			int	option;
			int	value;
		}	t_sooption;

		static const t_sooption	buildSocketOption(const int &level, const int &option,
			const int &value);

		/* Attributs */

		/* Constructors & Destructors */
		ASocket(void);

		ASocket(const ASocket &other);
		ASocket	&operator=(const ASocket &other);

		virtual ~ASocket(void);

		/* Member functions */

		// Getter
		const struct pollfd	&getPoll(void) const;
		struct sockaddr		*getAddress(void);

		const int			&getSocketFd(void) const;
		const std::string	getIP(void) const;
		uint16_t			getPort(void) const;

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */
		struct sockaddr_in	_address;
		struct pollfd		_poll;

		/* Member functions */
		void					handleSocketErrors(const int &statusCode);
		virtual void			setSocketOptions(void) = 0;

	private:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

};
