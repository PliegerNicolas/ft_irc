/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:56:13 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/15 11:56:52 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ClientSocket.hpp"
#include "Server.hpp"

// MACROS

class	Server;

class	Client
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		Client(Server &server);

		Client(const Client &other);
		Client	&operator=(const Client &other);

		virtual ~Client(void);

		/* Member functions */

		// Getter
		ClientSocket		&getSocket(void);

		const std::string	getMessage(const char delimiter);

		// Setter
		void				addToBuffer(const char *buffer, const size_t readBytes);

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		ClientSocket		_socket;

		std::string			_nickname;
		std::string			_username;
		std::string			_realname;

		std::string			_password; // encryption :(

		std::string			_buffer;

		/* Constructors & Destructors */
		Client(void);

		/* Member functions */
};
