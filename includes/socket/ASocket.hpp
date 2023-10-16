/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 13:23:07 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 20:06:28 by nicolas          ###   ########.fr       */
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

class	ASocket
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

		typedef std::vector<struct pollfd>	PollFds;
		typedef PollFds::iterator			PollFdsIt;
		typedef PollFds::const_iterator		PollFdsConstIt;

		/* Attributs */

		/* Constructors & Destructors */
		ASocket(void);

		ASocket(const ASocket &other);
		ASocket	&operator=(const ASocket &other);

		virtual ~ASocket(void);

		/* Member functions */

		// Getter
		const std::string		getIP(void) const;
		uint16_t				getPort(void) const;

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */
		PollFds					_pollFds;

		struct addrinfo			_hints;
		struct addrinfo			*_addrInfo;

		/* Member functions */
		void					handleSocketErrors(const int &statusCode);
		virtual void			setSocketOptions(void) = 0;

	private:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */
		struct addrinfo			*addrInfoDeepCopy(const ASocket &other);
};
