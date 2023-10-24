/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mfaucheu <mfaucheu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 13:30:05 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/24 12:55:46 by mfaucheu         ###   ########.fr       */
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

void				toggleBits(int &mask, const int &bits);
void				setBits(int &mask, const int &bits);
void				removeBits(int &mask, const int &bits);
