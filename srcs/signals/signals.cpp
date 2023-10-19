/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 15:28:51 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/19 15:47:50 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "signals/signals.hpp"

void	sigintHandler(int signal)
{
	if (!(signal == SIGINT || signal == SIGTERM))
		return ;

	g_serverExit = true;
	std::cout << "Exiting gracefuly..." << std::endl;
}
