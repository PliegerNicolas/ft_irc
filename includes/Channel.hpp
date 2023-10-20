/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:30:59 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/20 15:43:46 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "Server.hpp"
#include "Client.hpp"

// MACROS

class	Client;

class	Channel
{
	public:
		/* Typedefs */

		typedef enum ChannelStatus
		{
			PASSWORD_PROTECTED = (1 << 1),		// Password needed to access.
			RESTRICT_TOPIC = (1 << 2),			// Topic can only be modified by a moderator.
			INVITE_ONLY = (1 << 3),				// Can only be accessed if invited by someone
												// with invite privileges.
			NO_EXTERNAL_MESSAGES = (1 << 4),	// No messages from outside but announcements.
			SECRET = (1 << 5)					// Channel is hidden from channels list.
		}	t_channelPerms;

		typedef enum UserPermissions
		{
			KICK = (1 << 1),
			BAN = (1 << 2),
			INVITE = (1 << 3),
			TOPIC = (1 << 4),
			MODE = (1 << 5)
		}	t_userPerms;

		/* Attributs */

		/* Constructors & Destructors */
		Channel(const Client* channelCreator);

		Channel(const Channel &other);
		Channel	&operator=(const Channel &other);

		~Channel(void);

		/* Member functions */

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */
	private:
		/* Typedefs */

		typedef struct User
		{
			const Client*	client;
			size_t			permissionsMask;
		}	t_user;

		typedef std::vector<t_user>	Users;

		typedef Users::iterator		UsersIterator;

		/* Attributs */

		Users	_users;
		// Vector with banned IPs

		int		_userLimit;

		/* Constructors & Destructors */
		Channel(void);

		/* Member functions */
		t_user	createUser(const Client* client, const size_t &permissionsMask);

		// GETTERS
		size_t	getUserPerms(void);
		size_t	getHalfOpsPerms(void);
		size_t	getOpsPerms(void);
		size_t	getAdminPerms(void);

		// SETTERS
};
