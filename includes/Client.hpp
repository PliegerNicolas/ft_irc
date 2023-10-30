/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:56:13 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/30 12:15:55 by hania            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// INCLUDES

#include "socket/ClientSocket.hpp"
#include "Server.hpp"
#include "Channel.hpp"

#include <vector>
#include <map>

// MACROS

class	Server;
class	Channel;

class	Client
{
	public:
		/* Typedefs */
		typedef enum ServerPermissions
		{
			VERIFIED = (1 << 0),
			IDENTIFIED = (1 << 1)
		}	t_serverPermissions;

		typedef std::vector<Client*>			Clients;
		typedef std::map<std::string, Channel*>	Channels;

		typedef Clients::iterator				ClientsIterator;
		typedef Channels::iterator				ChannelsIterator;

		/* Attributs */

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

		void				addToJoinedChannels(Channel *channel);

		void				receiveMessage(const std::string &message) const;
		void				broadcastMessageToChannel(const Channel *channel,
								const std::string &message) const;
		//void				broadcastMessageToActiveChannel(const std::string &message) const;

		// Getter
		std::string			&getBuffer(void);
		int					getSocketFd(void) const;

		const std::string	&getNickname(void) const;
		const std::string	&getUsername(void) const;
		const std::string	&getRealname(void) const;
		short				&getConnectionRetries(void);
		int					getServerPermissions(void) const;
		Channels			&getJoinedChannels(void);
		Channel				*getActiveChannel(void);

		// Setter
		void				setNickname(const std::string &nickname);
		void				setUsername(const std::string &username);
		void				setRealname(const std::string &realname);
		void				setServerPermissions(const int &mask);
		void				setActiveChannel(Channel *channel);

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		ClientSocket		_clientSocket;
		Channel				*_activeChannel;
		Channels			_joinedChannels;

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
