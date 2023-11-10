/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:30:59 by nplieger          #+#    #+#             */
/*   Updated: 2023/11/10 15:08:37 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// INCLUDES

#include "Server.hpp"
#include "Client.hpp"

#include <vector>
#include <map>

// MACROS

#define MODE_CHANGED 1
#define MODE_UNCHANGED 0
#define MODE_INVALID -1

#define MODES_CHANNEL "tinmlkps"
#define MODES_USER "hoaq"

#define MAX_TOPIC_LEN 306
#define MAX_CHANNELNAME_LEN 50

class	Server;
class	Client;

class	Channel
{
	public:
		/* Typedefs */

		typedef enum ChannelMode
		{
			TOPIC_LOCK = (1 << 0),				// Topic can only be modified by a moderator (t)
			INVITE_ONLY = (1 << 1),				// Can only be accessed if invited by someone
												// with invite privileges (i)
			NO_EXTERNAL_MESSAGES = (1 << 2),	// No messages from outside but announcements (n)
			MODERATED = (1 << 3),				// Only users with voice or higher modes are allowed
												//to talk (PRIVMSG or NOTICE) (m)
			USER_LIMIT = (1 << 4),				// user limit is set (l)
			KEY_PASS = (1 << 5),				// Needs a password to join (k)
			PRIVATE = (1 << 6),					// Channel's Member list hidden (p)
			SECRET = (1 << 7)					// Channel is hidden from channels list
												// + disable NAMES (s)
		}	t_channelMode;

		typedef enum UserMode
		{
			VOICE = (1 << 2),					// Can speed in moderated channels.
			HALF_OPERATOR = (1 << 3),			// Become half channel operator.
			OPERATOR = (1 << 4),				// become channel operator.
			ADMIN = (1 << 5),					// become channel admin.
			OWNER = (1 << 6)					// become channel owner.
		}	t_userMode;

		typedef struct User
		{
			Client		*client;
			int			modesMask;
		}	t_user;

		typedef std::map<std::string, Channel*>	Channels;
		typedef std::vector<Client*>			Clients;
		typedef t_user							User;
		typedef std::vector<User>				Users;

		typedef Channels::iterator				ChannelsIterator;
		typedef Clients::iterator				ClientsIterator;
		typedef Users::iterator					UsersIterator;
		typedef Users::const_iterator			UsersConstIterator;

		/* Attributs */

		/* Constructors & Destructors */
		Channel(const std::string &name);

		Channel(const Channel &other);
		Channel	&operator=(const Channel &other);

		~Channel(void);

		/* Member functions */

		void				addUser(Client* client, const int &modesMask);
		void				addInvitation(Client *client);
		void				removeUser(const Client* client);
		void				removeInvitation(Client *client);
		bool				isFull(void) const;
		bool				isEmpty(void) const;
		bool				isInvited(Client *client);
		bool				isClientRegistered(const Client* client) const;

		bool				canKick(const Client *client);
		bool				canInvite(const Client *client);
		bool				canChangeTopic(const Client *client);

		int					addChannelMode(const char &mode, const std::string &argument);
		int					removeChannelMode(const char &mode);
		int					addUserMode(User *targetUser, const char &mode,
								const std::string &argument);
		int					removeUserMode(User *targetUser, const char &modes);

		// GETTERS

		const Users			&getUsers(void) const;
		User				*getUser(const std::string &nickname);

		const std::string	&getName(void) const;
		const std::string	&getTopic(void) const;
		const std::string	getUserLimit(void) const;
		const std::string	&getPassword(void) const;

		const std::string	getChannelModes(void) const;
		int					getChannelModesMask(void) const;
		const std::string	getUserModes(const User *targetUser) const;
		int					getUserModesMask(const User *targetUser) const;

		const std::string	getUserPrefix(User *targetUser) const;

		// SETTERS

		void				setTopic(const std::string &topic);
		void				setUserLimit(const std::string &userLimit);
		void				setPassword(const std::string &password);

		void				setChannelModesMask(const int &mask);
		void				setUserModesMask(User *targetUser, const int &mask);

		/* Static functions */

		static int			defaultUserPerms(void);
		static int			defaultHalfOpsPerms(void);
		static int			defaultOpsPerms(void);
		static int			defaultAdminPerms(void);
		static int			defaultOwnerPerms(void);

		static bool			isChannelMode(const char &mode);
		static bool			isUserMode(const char &mode);

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		std::string	_name;
		std::string	_topic;
		int			_modesMask;

		Users		_users;
		Clients		_invitedClients;

		int			_userLimit;
		std::string	_password;

		/* Constructors & Destructors */
		Channel(void);

		/* Member functions */
		t_user	createUser(Client* client, const int &permissionsMask);

		// GETTERS

		// SETTERS

		/* Static functions */

		static int			channelModeToMask(const char &mode);
		static std::string	channelMaskToModes(const int &mask);

		static int			userModeToMask(const char &mode);
		static std::string	userMaskToModes(const int &mask);
};
