/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:56:13 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/23 15:53:25 by nplieger         ###   ########.fr       */
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

		typedef enum ServerPermissions
		{
			VERIFIED = (1 << 0),
			IDENTIFIED = (1 << 1)
		}	t_serverPermissions;

		/* Constructors & Destructors */
		Client(const ASocket::t_socket &serverSocket);

		Client(const Client &other);
		Client	&operator=(const Client &other);

		virtual ~Client(void);

		/* Member functions */
		const struct pollfd	generatePollFd(void);
		void				closeSocketFd(void);

		int					readAndStoreFdBuffer(Server &server,
								const struct pollfd &pollFd);
		void				incrementConnectionRetries(void);

		// Getter
		std::string			&getBuffer(void);
		int					getSocketFd(void);

		const std::string	&getNickname(void) const;
		short				&getConnectionRetries(void);
		int					&getServerPermissions(void);

		// Setter
		void				setNickname(const std::string &nickname);
		void				setServerPermissions(const int &mask);

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

		int					_serverPermissions;
		std::string			_password; // encryption :(
		short				_connectionRetries;

		std::string			_nickname;
		std::string			_username;
		std::string			_realname;


		std::string			_messageBuffer;
		std::string			_messagePrefix;

		/* Constructors & Destructors */
		Client(void);

		/* Member functions */
};
