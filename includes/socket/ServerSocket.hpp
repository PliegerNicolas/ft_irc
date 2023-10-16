/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:14:59 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 20:28:03 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ASocket.hpp"

// MACROS

#define SERVOPTSIZE 6

class	ServerSocket: public ASocket
{
	public:
		/* Typedefs */

		typedef struct ServerConfig
		{
			int			domain;		// communication domain (IPv4, IPv6, AF_UNIX, ...)
			int			service;	// communication semantics (stream, dgream, raw, ...)
			int			protocol;	// transmission protocol (TCP, UDP, HTTP, ...)
			const char	*interface;	// IP address ("127.0.0.1", ...)
			const char	*port;		// Port (6667, 6697, ...)
		} t_serverconfig;

		static const t_serverconfig	buildServerConfig(const int &domain, const int &service,
			const int &protocol, const char *interface, const char *port);

		typedef std::vector<struct pollfd>	PollFds;
		typedef PollFds::iterator			PollFdsIt;
		typedef PollFds::const_iterator		PollFdsConstIt;

		/* Attributs */

		/* Constructors & Destructors */
		ServerSocket(void);
		ServerSocket(const t_serverconfig &serverConfig);
		//ServerSocket(const int &domain, const int &service, const int &protocol,
		//	const std::string &interface, const int &port);

		ServerSocket(const ServerSocket &other);
		ServerSocket	&operator=(const ServerSocket &other);

		virtual ~ServerSocket(void);

		/* Member functions */

		// Getter

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */
		void	launchServerSockets(const t_serverconfig &serverConfig);
		void	setSocketOptions(void);
};
