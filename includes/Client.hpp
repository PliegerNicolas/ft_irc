/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:56:13 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/08 00:35:35 by nicolas          ###   ########.fr       */
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

		/*
		typedef enum ClientMode
		{
			SSL_TLS = (1 << 1),					// connected in SSL mode. Not used in our case.
			INVISIBLE = (1 << 0)
		}	t_clientMode;
		*/

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

		void				receiveMessage(const std::string &message) const;
		void				broadcastMessageToChannel(const Channel *channel,
								const std::string &message) const;

		void				joinChannel(Channel *channel);
		void				quitChannel(Channel *channel);

		// Getter
		std::string			&getBuffer(void);
		int					getSocketFd(void) const;

		const std::string	&getNickname(void) const;
		const std::string	&getUsername(void) const;
		const std::string	&getHostname(void) const;
		const std::string	&getServername(void) const;
		const std::string	&getRealname(void) const;
		short				&getConnectionRetries(void);
		int					getServerPermissions(void) const;
		Channels			&getJoinedChannels(void);
		Channel				*getActiveChannel(void);

		// Setter
		void				setNickname(const std::string &nickname);
		void				setUsername(const std::string &username);
		void				setHostname(const std::string &hostname);
		void				setServername(const std::string &servername);
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
		std::string			_hostname;
		std::string			_originServername;
		std::string			_realname;

		std::string			_messageBuffer;

		/* Constructors & Destructors */
		Client(void);

		/* Member functions */
};
