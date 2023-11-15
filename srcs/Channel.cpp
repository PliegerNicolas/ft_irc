/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:50:37 by nplieger          #+#    #+#             */
/*   Updated: 2023/11/15 11:37:44 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"

/* Constructors & Destructors */

	/* Public */

Channel::Channel(const std::string &name):
	_name(truncate(name, MAX_CHANNELNAME_LEN)),
	_modesMask(NO_EXTERNAL_MESSAGES | TOPIC_LOCK),
	_userLimit(-1),
	_password("")
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default constructor called.";
		std::cout << WHITE << std::endl;
	}
	std::cout << GREEN << "Channel " << BGREEN <<_name << GREEN << " has been created" << WHITE << std::endl;
}

Channel::Channel(const Channel &other):
	_name(other._name),
	_topic(other._topic),
	_modesMask(other._modesMask),
	_users(other._users),
	_invitedClients(other._invitedClients),
	_userLimit(other._userLimit),
	_password(other._password)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: copy constructor called.";
		std::cout << WHITE << std::endl;
	}
	std::cout << GREEN << "Channel " << BGREEN <<_name << GREEN << " has been created" << WHITE << std::endl;
}

Channel	&Channel::operator=(const Channel &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: assignment operator called.";
		std::cout << WHITE << std::endl;
	}

	if (this != &other)
	{
		_name = other._name;
		_topic = other._topic;
		_modesMask = other._modesMask;
		_users = other._users;
		_invitedClients = other._invitedClients;
		_userLimit = other._userLimit;
		_password = other._password;
	}

	return (*this);
}

Channel::~Channel(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default destructor called.";
		std::cout << WHITE << std::endl;
	}

	_users.clear();
	_invitedClients.clear();
	std::cout << GREEN << "Channel " << BGREEN <<_name << GREEN << " has been deleted" << WHITE << std::endl;
}
	/* Protected */
	/* Private */

Channel::Channel(void):
	_name(""),
	_modesMask(0),
	_userLimit(-1),
	_password("")
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default constructor called.";
		std::cout << WHITE << std::endl;
	}
}

/* Member functions */

	/* Public */

void	Channel::addUser(Client* client, const int &permissionsMask)
{
	_users.push_back(createUser(client, permissionsMask));
}

void	Channel::addInvitation(Client *client)
{
	_invitedClients.push_back(client);
}

void	Channel::removeUser(const Client* client)
{
	UsersIterator	it = _users.begin();

	for (; it != _users.end() && it->client != client; ++it);

	if (it != _users.end())
		_users.erase(it);
}

void	Channel::removeInvitation(Client *client)
{
	ClientsIterator	it = _invitedClients.begin();

	for (; it != _invitedClients.end() && client != *it; ++it);

	if (it != _invitedClients.end())
		_invitedClients.erase(it);
}

bool	Channel::isFull(void) const
{
	if (_userLimit == -1 || _userLimit > static_cast<int>(_users.size()))
		return (false);
	return (true);
}

bool	Channel::isEmpty(void) const
{
	if (_users.size() == 0)
		return (true);
	return (false);
}

bool	Channel::isInvited(Client *client)
{
	ClientsIterator	it = _invitedClients.begin();

	for (; it != _invitedClients.end() && *it != client; ++it);

	if (it != _invitedClients.end())
		return (true);
	return (false);
}

bool	Channel::isOwner(const Client *client)
{
	const User	*user = getUser(client->getNickname());

	if (user && areBitsSet(user->modesMask, OWNER))
		return (true);
	return (false);
}

bool	Channel::isClientRegistered(const Client* client) const
{
	for (UsersConstIterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (client == it->client)
			return (true);
	}

	return (false);
}

bool	Channel::canKick(const Client *source, const Client *target)
{
	const User	*sourceUser = Channel::getUser(source->getNickname());
	const User	*targetUser = Channel::getUser(target->getNickname());

	if (sourceUser && targetUser)
	{
		const int	&sourceModesMask = getUserModesMask(sourceUser);
		const int	&targetModesMask = getUserModesMask(targetUser);

		if (areBitsSet(source->getClientModesMask(), Client::OPERATOR))
			return (true);
		else if (areBitsSet(sourceModesMask, OWNER)
			&& !isAtLeastOneBitSet(targetModesMask, OWNER))
			return (true);
		else if (areBitsSet(sourceModesMask, ADMIN)
			&& !isAtLeastOneBitSet(targetModesMask, OWNER | ADMIN))
			return (true);
		else if (areBitsSet(sourceModesMask, OPERATOR)
			&& !isAtLeastOneBitSet(targetModesMask, OWNER | ADMIN | OPERATOR))
			return (true);
		else if (areBitsSet(sourceModesMask, HALF_OPERATOR)
			&& !isAtLeastOneBitSet(targetModesMask, OWNER | ADMIN | OPERATOR | HALF_OPERATOR))
			return (true);
	}
	return (false);
}

bool	Channel::canInvite(const Client *client)
{
	if (areBitsSet(client->getClientModesMask(), Client::OPERATOR))
		return (true);

	const User	*user = Channel::getUser(client->getNickname());

	if (user && isAtLeastOneBitSet(getUserModesMask(user),
		HALF_OPERATOR | OPERATOR | ADMIN | OWNER))
		return (true);
	return (false);
}

bool	Channel::canChangeTopic(const Client *client)
{
	if (areBitsSet(client->getClientModesMask(), Client::OPERATOR)
		|| areBitsNotSet(_modesMask, TOPIC_LOCK))
		return (true);

	const User	*user = Channel::getUser(client->getNickname());

	if (user && isAtLeastOneBitSet(getUserModesMask(user),
		OPERATOR | ADMIN | OWNER))
		return (true);
	return (false);
}

bool	Channel::canTalk(const Client *client)
{
	if (areBitsNotSet(_modesMask, MODERATED)
		|| areBitsSet(client->getClientModesMask(), Client::OPERATOR))
		return (true);

	const User	*user = Channel::getUser(client->getNickname());

	if (user && isAtLeastOneBitSet(getUserModesMask(user),
		VOICE | OPERATOR | ADMIN | OWNER))
		return (true);
	return (false);
}

bool	Channel::canUpdateModes(const Client *client)
{
	if (areBitsSet(client->getClientModesMask(), Client::OPERATOR))
		return (true);

	const User	*user = Channel::getUser(client->getNickname());

	if (user && isAtLeastOneBitSet(getUserModesMask(user),
		OPERATOR | ADMIN | OWNER))
		return (true);
	return (false);
}



int	Channel::addChannelMode(const char &mode, const std::string &argument)
{
	int	mask = Channel::channelModeToMask(mode);

	if (!mask)
		return (MODE_INVALID);
	else if (areBitsSet(_modesMask, mask)
		&& mask != KEY_PASS && mask != USER_LIMIT)
		return (MODE_UNCHANGED);
	else
	{
		if (mask == KEY_PASS)
		{
			setPassword(argument);

			if (areBitsSet(_modesMask, USER_LIMIT) && _password.empty())
				removeBits(_modesMask, mask);
			else
				setBits(_modesMask, mask);
		}
		else if (mask == USER_LIMIT)
		{
			setUserLimit(argument);

			if (areBitsSet(_modesMask, USER_LIMIT) && _userLimit == -1)
				removeBits(_modesMask, mask);
			else
				setBits(_modesMask, mask);
		}
		else
			setBits(_modesMask, mask);

		return (MODE_CHANGED);
	}
}

int	Channel::removeChannelMode(const char &mode)
{
	int	mask = Channel::channelModeToMask(mode);

	if (!mask)
		return (MODE_INVALID);
	else if (areBitsNotSet(_modesMask, mask))
		return (MODE_UNCHANGED);
	else
		return (removeBits(_modesMask, mask), MODE_CHANGED);
}

int	Channel::addUserMode(User *targetUser, const char &mode, const std::string &argument)
{
	if (!targetUser)
		return (MODE_UNCHANGED);

	(void)argument;

	int	mask = Channel::userModeToMask(mode);

	if (!mask)
		return (MODE_INVALID);
	else if (areBitsSet(targetUser->modesMask, mask))
		return (MODE_UNCHANGED);
	else
		return (setBits(targetUser->modesMask, mask), MODE_CHANGED);
}

int	Channel::removeUserMode(User *targetUser, const char &mode)
{
	if (!targetUser)
		return (MODE_UNCHANGED);

	int	mask = Channel::userModeToMask(mode);

	if (!mask)
		return (MODE_INVALID);
	else if (areBitsNotSet(targetUser->modesMask, mask))
		return (MODE_UNCHANGED);
	else
		return (removeBits(targetUser->modesMask, mask), MODE_CHANGED);
}

Channel::User	*Channel::findFirstHighestPrivilege(void)
{
	UsersIterator	it = _users.begin();
	User			*user = NULL;

	if (it != _users.end())
	{
		user = &(*it);
		it++;
	}

	for (; it != _users.end(); ++it)
	{
		if (it->modesMask > user->modesMask)
			user = &(*it);
	}

	return (user);
}

	/* Protected */
	/* Private */

Channel::t_user	Channel::createUser(Client* client, const int  &modesMask)
{
	t_user	user;

	user.client = client;
	user.modesMask = modesMask;

	return (user);
}

/* Getters */

	/* Public */

const std::string	&Channel::getName(void) const
{
	return (_name);
}

const std::string	&Channel::getTopic(void) const
{
	return (_topic);
}

const std::string	Channel::getUserLimit(void) const
{
	std::ostringstream	oss;

	oss << _userLimit;
	return (oss.str());
}

const std::string &Channel::getPassword(void) const
{
	return (_password);
}

Channel::User	*Channel::getUser(const std::string &nickname)
{
	UsersIterator	it = _users.begin();

	for (; it != _users.end() && it->client->getNickname() != nickname; ++it);

	if (it != _users.end())
		return (&(*it));
	return (NULL);
}

const Channel::Users	&Channel::getUsers(void) const
{
	return (_users);
}

const std::string	Channel::getChannelModes(void) const
{
	return (Channel::channelMaskToModes(_modesMask));
}

int	Channel::getChannelModesMask(void) const
{
	return (_modesMask);
}

const std::string	Channel::getUserModes(const User *targetUser) const
{
	if (!targetUser)
		return ("");
	return (Channel::userMaskToModes(targetUser->modesMask));
}

int	Channel::getUserModesMask(const User *targetUser) const
{
	if (!targetUser)
		return (0);
	return (targetUser->modesMask);
}

const std::string	Channel::getUserPrefix(const User *targetUser) const
{
	std::string	prefix;

	if (!targetUser)
		return (prefix);

	if (areBitsSet(targetUser->client->getClientModesMask(), Client::OPERATOR)
		|| isAtLeastOneBitSet(getUserModesMask(targetUser), OWNER | ADMIN | OPERATOR))
		prefix = "@";
	else if (areBitsSet(getUserModesMask(targetUser), HALF_OPERATOR))
		prefix = "%";
	else if (areBitsSet(getUserModesMask(targetUser), Channel::VOICE))
		prefix = "+";

	return (prefix);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */

void	Channel::setTopic(const std::string &topic)
{
	_topic = truncate(topic, MAX_TOPIC_LEN);
}

void	Channel::setUserLimit(const std::string &userLimit)
{
	int	limit;
	std::istringstream	iss(userLimit);

	if (iss >> limit)
	{
		if (limit <= 0)
			limit = -1;
		_userLimit = limit;
	}
	else
	{
		// Error message ? Keep the old value.
	}
}

void	Channel::setPassword(const std::string &password)
{
	if (!password.empty())
		_password = password;
}

void	Channel::setChannelModesMask(const int &mask)
{
	setBits(_modesMask, mask);
}

void	Channel::setUserModesMask(User *targetUser, const int &mask)
{
	if (!targetUser)
		return ;

	setBits(targetUser->modesMask, mask);
}

	/* Protected */
	/* Private */

/* Static functions */

	/* Public */

int	Channel::defaultUserPerms(void)
{
	return (0);
}

int	Channel::defaultHalfOpsPerms(void)
{
	return (HALF_OPERATOR);
}

int	Channel::defaultOpsPerms(void)
{
	return (VOICE | OPERATOR);
}

int	Channel::defaultAdminPerms(void)
{
	return (VOICE | ADMIN);
}

int	Channel::defaultOwnerPerms(void)
{
	return (VOICE | OWNER);
}

bool	Channel::isChannelMode(const char &mode)
{
	if (strchr(MODES_CHANNEL, mode) != NULL)
		return (true);
	return (false);
}

bool	Channel::isUserMode(const char &mode)
{
	if (strchr(MODES_USER, mode) != NULL)
		return (true);
	return (false);
}


	/* Protected */
	/* Private */

int	Channel::channelModeToMask(const char &mode)
{
	switch (mode)
	{
		case 't':
			return (TOPIC_LOCK);
		case 'i':
			return (INVITE_ONLY);
		case 'n':
			return (NO_EXTERNAL_MESSAGES);
		case 'm':
			return (MODERATED);
		case 'l':
			return (USER_LIMIT);
		case 'k':
			return (KEY_PASS);
		case 'p':
			return (PRIVATE);
		case 's':
			return (SECRET);
		default:
			return (0);
	}
}

std::string	Channel::channelMaskToModes(const int &mask)
{
	std::string	modes;
	const char	*channelModes = MODES_CHANNEL;

	for (size_t shift = 0; shift < strlen(channelModes); ++shift)
	{
		if((mask >> shift) & 1)
			modes += channelModes[shift];
	}

	if (modes.empty())
		return ("");
	return ("+" + modes);
}

int	Channel::userModeToMask(const char &mode)
{
	switch (mode)
	{
		case 'v':
			return (VOICE);
		case 'h':
			return (HALF_OPERATOR);
		case 'o':
			return (OPERATOR);
		case 'a':
			return (ADMIN);
		case 'q':
			return (OWNER);
		default:
			return (0);
	}
}

std::string	Channel::userMaskToModes(const int &mask)
{
	std::string	modes;
	const char	*userModes = MODES_USER;

	for (size_t shift = 0; shift < strlen(userModes); ++shift)
	{
		if((mask >> shift) & 1)
			modes += userModes[shift];
	}

	if (modes.empty())
		return ("");
	return ("+" + modes);
}

bool	Channel::isValidName(const std::string &name)
{
	if (!name.empty() && name[0] == '#' && name[1])
		return (true);
	return (false);
}
