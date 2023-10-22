/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 13:30:05 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/23 01:27:40 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

#include <string>

void				removeLeadingWhitespaces(std::string &str, const std::string &delimiter);
size_t				findLastWordEnd(const std::string &str, const size_t &strLen);
void				capitalizeString(std::string &str);

std::string			getNextWord(std::string &str, const std::string &delimiter);
