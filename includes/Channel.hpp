/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:30:59 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/24 16:27:14 by mfaucheu         ###   ########.fr       */
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
			PASSWORD_PROTECTED = (1 << 0),		// Password needed to access.
			RESTRICT_TOPIC = (1 << 1),			// Topic can only be modified by a moderator.
			INVITE_ONLY = (1 << 2),				// Can only be accessed if invited by someone
												// with invite privileges.
			NO_EXTERNAL_MESSAGES = (1 << 3),	// No messages from outside but announcements.
			SECRET = (1 << 4)					// Channel is hidden from channels list.
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
			const Client*	client;
			size_t			permissionsMask;
		}	t_user;

		/* Attributs */

		/* Constructors & Destructors */
		Channel(const Client* channelCreator);

		Channel(const Channel &other);
		Channel	&operator=(const Channel &other);

		~Channel(void);

		/* Member functions */

		void	addUser(const Client* client, const int &mask);
		bool	isUserRegistered(const Client* client) const;

		// GETTERS

		int	getUserPerms(void);
		int	getHalfOpsPerms(void);
		int	getOpsPerms(void);
		int	getAdminPerms(void);

		// SETTERS

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */
	private:
		/* Typedefs */

		typedef std::vector<t_user>	Users;

		typedef Users::iterator		UsersIterator;
		typedef Users::const_iterator	UsersConstIterator;

		/* Attributs */

		Users	_users;
		int		_userLimit;

		/* Constructors & Destructors */
		Channel(void);

		/* Member functions */
		t_user	createUser(const Client* client, const size_t &permissionsMask);

		// GETTERS

		// SETTERS
};
