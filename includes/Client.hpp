/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:56:13 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/10 14:45:09 by nicolas          ###   ########.fr       */
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

#define MODE_CHANGED 1
#define MODE_UNCHANGED 0
#define MODE_INVALID -1

#define MODES_CLIENT "iwoxz"

#define MAX_USERNAME_LEN 18
#define MAX_NICKNAME_LEN 9

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

		typedef enum ClientMode
		{
			INVISIBLE = (1 << 0),				// Invisible in channel user lists or queries (i)
			WALLOPS = (1 << 1),					// Allow server-wide notifications reception (w)
			OPERATOR = (1 << 2),				// General operator privileges (o)
			SSL_TLS = (1 << 3),					// connected in SSL mode (x)
			HIDE_HOSTNAME = (1 << 4)			// Secure connection. Hides user's hostname (z)
		}	t_clientMode;

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

		int					addClientMode(const char &mode, const std::string &argument);
		int					removeClientMode(const char &mode);

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

		const std::string	getClientModes(void) const;
		int					getClientModesMask(void) const;

		const std::string	getPrefix(void) const;

		// Setter
		void				setNickname(const std::string &nickname);
		void				setUsername(const std::string &username);
		void				setHostname(const std::string &hostname);
		void				setServername(const std::string &servername);
		void				setRealname(const std::string &realname);
		void				setActiveChannel(Channel *channel);

		void				setServerPermissions(const int &mask);
		void				setClientModesMask(const int &mask);

		/* Static */
		static bool			isClientMode(const char &mode);
		static bool			isValidNickname(const std::string &nickname);

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
		int					_modesMask;

		std::string			_nickname;
		std::string			_username;
		std::string			_hostname;
		std::string			_originServername;
		std::string			_realname;

		std::string			_messageBuffer;

		/* Constructors & Destructors */
		Client(void);

		/* Member functions */

		/* Static */
		static int			clientModeToMask(const char &mode);
		static std::string	clientMaskToModes(const int &mask);
};
