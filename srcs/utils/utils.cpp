/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/29 11:14:21 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/06 16:08:08 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/utils.hpp"

void	removeLeadingWhitespaces(std::string &str, const std::string &delimiter)
{
	size_t	endPos = str.find(delimiter);
	size_t	i = 0;

	if (endPos == std::string::npos)
		endPos = str.length();

	while (i < endPos && std::isspace(str[i]))
		i++;
	str.erase(0, i);
}

size_t	findLastChar(const std::string &str, const size_t &strLen)
{
	size_t	pos = strLen;
	size_t	middle = strLen / 2;

	while (pos > middle && !isspace(str[pos]))
		pos--;
	while (pos > middle && isspace(str[pos]))
		pos--;
	if (pos == middle)
		return (strLen);
	return (pos + 1);
}

void	capitalizeString(std::string &str)
{
	for (std::string::size_type i = 0; i < str.length(); i++)
	{
		if (islower(str[i]))
			str[i] = toupper(str[i]);
	}
}

std::string	getNextWord(std::string &str, const std::string &delimiter)
{
	removeLeadingWhitespaces(str, delimiter);

	if (str.empty())
		return (std::string());

	size_t	i = 0;
	while (str[i] && !std::isspace(str[i]))
		i++;

	std::string	word = str.substr(0, i);
	str.erase(0, i);

	removeLeadingWhitespaces(str, delimiter);

	return (word);
}

std::string	truncate(const std::string &str, const size_t &width)
{
	if (str.length() > width)
		return (str.substr(0, width));
	return (str);
}

void	stripDuplicateChars(std::string &str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		size_t j = i + 1;
		for (; j < str.length(); j++)
		{
			if (str[i] == str[j])
				str.erase(j);
		}
	}
}

long int	validatePresenceInString(const std::string &str, const std::string &charset)
{
	long int	charPresenceValidations = 0;

	for (size_t i = 0; i < charset.length(); i++)
	{
		for (size_t j = 0; j < str.length(); j++)
		{
			if (charset[i] == str[j])
			{
				charPresenceValidations++;
				continue ;
			}
		}
	}
	return (charPresenceValidations);
}

/* ************************************************************************** */
/* *                            BitMasks handling                           * */
/* ************************************************************************** */

bool	areBitsSet(const int &mask, const int &bits)
{
	if ((mask & bits) == bits)
		return (true);
	return (false);
}

bool	areBitsNotSet(const int &mask, const int &bits)
{
	if ((mask & bits) != bits)
		return (true);
	return (false);
}
