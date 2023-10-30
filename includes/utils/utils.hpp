/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 13:30:05 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/30 13:26:44 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

void				removeLeadingWhitespaces(std::string &str, const std::string &delimiter);
size_t				findLastChar(const std::string &str, const size_t &strLen);
void				capitalizeString(std::string &str);

std::string			getNextWord(std::string &str, const std::string &delimiter);

std::string			truncate(const std::string &str, const size_t &width);

/* ************************************************************************** */
/* *                            BitMasks handling                           * */
/* ************************************************************************** */

bool				areBitsSet(const int &mask, const int &bits);
bool				areBitsNotSet(const int &mask, const int &bits);

inline void			toggleBits(int &mask, const int &bits);
inline void			setBits(int &mask, const int &bits);
inline void			removeBits(int &mask, const int &bits);

#include "utils/utils.ipp"
