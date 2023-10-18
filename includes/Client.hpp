/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:56:13 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/18 12:12:52 by nplieger         ###   ########.fr       */
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
		Client(const ASocket::t_socket &serverSocket);

		Client(const Client &other);
		Client	&operator=(const Client &other);

		virtual ~Client(void);

		/* Member functions */
		const struct pollfd	generatePollFd(void);

		// Getter
		std::string			getMessage(const char delimiter);

		// Setter
		void				addToBuffer(const char *buffer, const size_t readBytes);

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		ClientSocket		_clientSocket;

		std::string		_nickname;
		std::string		_username;
		std::string		_realname;
		std::string		_password; // encryption :(

		std::string		_buffer;
		std::string		_prefix;

		/* Constructors & Destructors */
		Client(void);

		/* Member functions */
};
