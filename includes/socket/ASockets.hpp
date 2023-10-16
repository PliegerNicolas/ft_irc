/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASockets.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 13:23:07 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 22:33:33 by nicolas          ###   ########.fr       */
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
#include <cstdlib>
#include <cerrno>

#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <vector>

// MACROS

#define MAX_PORT 65535
#define MIN_PORT 0

class	ASockets
{
	public:
		/* Typedefs */
		typedef struct SocketOption
		{
			int	level;
			int	option;
			int	value;
		}	t_sooption;

		static const t_sooption	buildSocketOption(const int &level, const int &option,
			const int &value);

		typedef std::pair<int, struct addrinfo>		SocketPair;
		typedef std::vector<SocketPair>				Sockets;
		typedef Sockets::iterator					SocketsIt;
		typedef Sockets::const_iterator				SocketsConstIt;

		/* Attributs */

		/* Constructors & Destructors */
		ASockets(void);

		ASockets(const ASockets &other);
		ASockets	&operator=(const ASockets &other);

		virtual ~ASockets(void);

		/* Member functions */

		// Getter
		const std::string	getIP(const int &fd) const;
		const std::string	getPort(const int &fd) const;

		//const std::string		getIP(void) const;
		//uint16_t				getPort(void) const;

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */
		Sockets				_sockets;

		/* Member functions */
		virtual void			setSocketOptions(void) = 0;
		void					handleSocketErrors(const int &statusCode);

	private:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */
};
