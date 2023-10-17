/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSockets.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:16:24 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/17 18:47:16 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ASocket.hpp"
#include "socket/ServerSockets.hpp"

// MACROS

#define CLIENTOPTSIZE 6

class	ClientSockets: public ASocket
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		ClientSockets(void);
		ClientSockets(const ASocket::t_socket &serverSocket);

		ClientSockets(const ClientSockets &other);
		ClientSockets	&operator=(const ClientSockets &other);

		virtual ~ClientSockets(void);

		/* Member functions */

		// Getter

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		ASocket::t_socket	_socket;

		/* Constructors & Destructors */

		/* Member functions */
		void	setSocketOptions(void);
};
