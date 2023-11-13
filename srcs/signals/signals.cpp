/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 15:28:51 by nplieger          #+#    #+#             */
/*   Updated: 2023/11/14 00:22:32 by nicolas          ###   ########.fr       */
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

void	sigpipeHandler(int signal)
{
	if (signal != SIGPIPE)
		return ;

	char errMessage[256];
	ssize_t		putBytes;

	if (strerror_r(errno, errMessage, sizeof(errMessage)) != 0)
		errMessage[0] = '\0';
	putBytes = write(STDERR_FILENO, errMessage, strlen(errMessage));
	(void)putBytes;
}
