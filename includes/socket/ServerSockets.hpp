/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSockets.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:14:59 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/12 13:57:13 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ASocket.hpp"

// MACROS

#define SERVOPTSIZE 6

class	ServerSockets: public ASocket
{
	public:
		/* Typedefs */

		typedef struct ServerConfig
		{
			int			domain;		// communication domain (IPv4, IPv6, AF_UNIX, ...)
			int			service;	// communication semantics (stream, dgream, raw, ...)
			int			protocol;	// transmission protocol (TCP, UDP, HTTP, ...)
			const char	*interface;	// IP address or hostname ("127.0.0.1", "www.google.com", ...)
			const char	*port;		// Port (6667, 6697, ...)
		} t_serverconfig;

		static const t_serverconfig	buildServerConfig(const int &domain, const int &service,
			const int &protocol, const char *interface, const char *port);

		typedef std::vector<t_socket>	Sockets;
		typedef Sockets::iterator		SocketsIt;
		typedef Sockets::const_iterator	SocketsConstIt;

		/* Attributs */

		/* Constructors & Destructors */
		ServerSockets(void);
		ServerSockets(const t_serverconfig &serverConfig);

		ServerSockets(const ServerSockets &other);
		ServerSockets	&operator=(const ServerSockets &other);

		virtual ~ServerSockets(void);

		/* Member functions */
		const struct pollfd	generatePollFd(void);

		// Getter
		const Sockets		&getSockets(void) const;
		const std::string	&getHostname(void) const;

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		Sockets				_sockets;
		const std::string	_hostname;

		/* Constructors & Destructors */

		/* Member functions */
		void	launchServerSockets(const t_serverconfig &serverConfig);
		void	setSocketOptions(void);
		void	verifyPort(const char *strPort);

		void	handleErrors(const int &statusCode);
};
