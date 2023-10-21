/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 13:30:31 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/21 18:42:56 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "utils/Utils.hpp"

void	removeLeadingWhitespaces(std::string &str)
{
	size_t	i = 0;

	while (i < str.length() && isspace(str[i]))
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

std::string	getNextWord(std::string &str)
{
	size_t	pos = 0;
	std::string	word;

	if (str.empty())
		return (word);

	while (str[pos] && !isspace(str[pos]))
		pos++;

	word = str.substr(0, pos);
	str.erase(0, pos);
	removeLeadingWhitespaces(str);
	return (word);
}
