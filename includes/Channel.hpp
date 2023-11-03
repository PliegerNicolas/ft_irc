/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:30:59 by nplieger          #+#    #+#             */
/*   Updated: 2023/11/03 01:59:14 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// INCLUDES

#include "Server.hpp"
#include "Client.hpp"

#include <vector>
#include <map>

// MACROS

#define MODE_CHARACTERS "tnimpsklcr"

class	Server;
class	Client;

class	Channel
{
	public:
		/* Typedefs */

		typedef enum ChannelMode
		{
			TOPIC_LOCK = (1 << 0),				// Topic can only be modified by a moderator.
			INVITE_ONLY = (1 << 1),				// Can only be accessed if invited by someone
												// with invite privileges.
			NO_EXTERNAL_MESSAGES = (1 << 2),	// No messages from outside but announcements.
			MODERATED = (1 << 3),				// Only users with voice or higher modes are allowed
												//to talk (PRIVMSG or NOTICE)
			USER_LIMIT = (1 << 4),				// user limit is set.
			KEY_PASS = (1 << 5),				// Needs a password to join.
			PRIVATE = (1 << 6),					// Hidden from channels list.
			SECRET = (1 << 7)					// Channel is hidden from channels list
												// + disable NAMES.
		}	t_channelPerms;

		typedef enum UserMode
		{
			WALLOPS = (1 << 0),
			SERVER_NOTICE = (1 << 1),
			SSL_TLS = (1 << 2),
			INVISIBLE = (1 << 3),
			VOICE = (1 << 4),
			HALF_OPERATOR = (1 << 5),
			OPERATOR = (1 << 6),
			ADMIN = (1 << 7),
			OWNER = (1 << 8)
		}	t_mode;

		typedef struct User
		{
			Client		*client;
			size_t		modesMask;
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

		// GETTERS

		const std::string	&getName(void) const;
		const std::string	&getTopic(void) const;
		const Users			&getUsers(void) const;
		User				*getUser(const std::string &nickname);

		const int			&getModeMask(void) const; // temp ?

		// SETTERS

		void				setTopic(const std::string &topic);

		void				setModeMask(const int &mask); // temp ?

		/* Static functions */

		static int			defaultUserPerms(void);
		static int			defaultHalfOpsPerms(void);
		static int			defaultOpsPerms(void);
		static int			defaultAdminPerms(void);
		static int			defaultOwnerPerms(void);

		static int			channelModesToMask(const std::string &modes);
		static std::string	channelMaskToModes(const int &mask);

		static int			userModesToMask(const std::string &modes);
		static std::string	userMaskToModes(const int &mask);

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */
		std::string	_name;
		std::string	_topic;

		Users		_users;
		Clients		_invitedClients;

		int			_userLimit;
		int			_modeMask;

		/* Constructors & Destructors */
		Channel(void);

		/* Member functions */
		t_user	createUser(Client* client, const int &permissionsMask);

		// GETTERS

		// SETTERS
};
