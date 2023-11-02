/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:30:59 by nplieger          #+#    #+#             */
/*   Updated: 2023/11/02 14:19:49 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// INCLUDES

#include "Server.hpp"
#include "Client.hpp"

#include <vector>
#include <map>

// MACROS

class	Server;
class	Client;

class	Channel
{
	public:
		/* Typedefs */

		typedef enum ChannelStatus
		{
			RESTRICT_TOPIC = (1 << 0),			// Topic can only be modified by a moderator.
			INVITE_ONLY = (1 << 1),				// Can only be accessed if invited by someone
												// with invite privileges.
			NO_EXTERNAL_MESSAGES = (1 << 2),	// No messages from outside but announcements.
			SECRET = (1 << 3)					// Channel is hidden from channels list.
		}	t_channelPerms;

		typedef enum UserPermissions
		{
			KICK = (1 << 0),
			BAN = (1 << 1),
			INVITE = (1 << 2),
			TOPIC = (1 << 3),
			MODE = (1 << 4)
		}	t_userPerms;

		typedef struct User
		{
			Client*	client;
			size_t	permissionsMask;
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

		bool				isClientRegistered(const Client* client) const;
		void				addUser(Client* client, const int &mask);
		void				addInvitation(Client *client);
		void				removeUser(const Client* client);
		void				removeInvitation(Client *client);
		bool				isFull(void) const;
		bool				isEmpty(void) const;
		bool				isInvited(Client *client);

		// GETTERS

		const std::string	&getName(void) const;
		const std::string	&getTopic(void) const;
		const Users			&getUsers(void) const;
		User				*getUser(const std::string &nickname);
		const int			&getModeMask(void) const;

		int					getUserPerms(void);
		int					getHalfOpsPerms(void);
		int					getOpsPerms(void);
		int					getAdminPerms(void);

		// SETTERS

		void				setTopic(const std::string &topic);
		void				setModeMask(const int &mask);

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
