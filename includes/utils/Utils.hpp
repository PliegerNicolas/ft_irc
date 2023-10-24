/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 13:30:05 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/24 12:14:08 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

#include <string>

void				removeLeadingWhitespaces(std::string &str, const std::string &delimiter);
size_t				findLastChar(const std::string &str, const size_t &strLen);
void				capitalizeString(std::string &str);

std::string			getNextWord(std::string &str, const std::string &delimiter);
