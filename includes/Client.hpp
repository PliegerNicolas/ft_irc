/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:56:13 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/22 02:54:54 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ClientSocket.hpp"
#include "Server.hpp"
#include "Channel.hpp"

// MACROS

class	Server;
class	Channel;

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
		int					readAndStoreFdBuffer(Server &server,
								const struct pollfd &pollFd);
		void				closeSocketFd(void);

		// Getter
		std::string			&getBuffer(void);

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Typedefs */
		typedef std::vector<Channel*>	Channels;

		typedef Channels::iterator		ChannelsIterator;

		/* Attributs */
		ClientSocket		_clientSocket;
		Channel				*_currentChannel;
		Channels			_channels;

		//char				_nickname[10];
		std::string			_username;
		std::string			_realname;
		std::string			_password; // encryption :(

		std::string			_messageBuffer;
		std::string			_messagePrefix;

		/* Constructors & Destructors */
		Client(void);

		/* Member functions */
};
