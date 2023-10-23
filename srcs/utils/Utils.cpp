/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 13:30:31 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/23 11:27:25 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "utils/Utils.hpp"

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

size_t	findLastWordEnd(const std::string &str, const size_t &strLen)
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
