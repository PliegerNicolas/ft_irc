/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 15:27:28 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/27 20:07:04 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include <iostream>
#include <csignal>
#include <cstring>

// GLOBAL VARIABLE

/**
 * g_serverExit permits exiting the server cleanly when set on true.
**/
extern volatile sig_atomic_t	g_serverExit;

void	sigintHandler(int signal);
