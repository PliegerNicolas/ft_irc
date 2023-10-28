/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 15:28:51 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/27 20:11:30 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "signals/signals.hpp"

void	sigintHandler(int signal)
{
	if (!(signal == SIGINT || signal == SIGTERM))
		return ;

	const char	*exitMessage = "Exiting gracefuly...\n";
	ssize_t		putBytes;
	g_serverExit = true;

	putBytes = write(STDOUT_FILENO, exitMessage, strlen(exitMessage));
	(void)putBytes;
}
