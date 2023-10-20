/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 15:28:51 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/20 17:52:15 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "signals/signals.hpp"

void	sigintHandler(int signal)
{
	if (!(signal == SIGINT || signal == SIGTERM))
		return ;

	g_serverExit = true;

	const char	*exitMessage = "Exiting gracefuly...";
	write(STDOUT_FILENO, exitMessage, sizeof(exitMessage));
}
