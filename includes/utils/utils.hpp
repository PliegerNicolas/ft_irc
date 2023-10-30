/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 13:30:05 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/30 11:50:35 by hania            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

void				removeLeadingWhitespaces(std::string &str, const std::string &delimiter);
size_t				findLastChar(const std::string &str, const size_t &strLen);
void				capitalizeString(std::string &str);

std::string			getNextWord(std::string &str, const std::string &delimiter);

/* ************************************************************************** */
/* *                            BitMasks handling                           * */
/* ************************************************************************** */

bool				areBitsSet(const int &mask, const int &bits);
bool				areBitsNotSet(const int &mask, const int &bits);

inline void			toggleBits(int &mask, const int &bits);
inline void			setBits(int &mask, const int &bits);
inline void			removeBits(int &mask, const int &bits);

std::string			truncateStr(std::string str, size_t width);

#include "utils/utils.tpp"
