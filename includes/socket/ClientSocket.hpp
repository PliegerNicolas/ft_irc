/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:16:24 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/19 16:09:52 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ASocket.hpp"
#include "socket/ServerSockets.hpp"

// MACROS

#define CLIENTOPTSIZE 6

class	ClientSocket: public ASocket
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		ClientSocket(void);
		ClientSocket(const ASocket::t_socket &serverSocket);

		ClientSocket(const ClientSocket &other);
		ClientSocket	&operator=(const ClientSocket &other);

		virtual ~ClientSocket(void);

		/* Member functions */

		// Getter
		const ASocket::t_socket	getSocket(void) const;

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
		void	handleErrors(const int &statusCode);
};
