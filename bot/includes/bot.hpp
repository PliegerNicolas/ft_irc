/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/12 13:41:00 by hania             #+#    #+#             */
/*   Updated: 2023/11/12 17:28:37 by hania            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <signal.h>
#include <poll.h>
#include <vector>

void						send_msg(int sd, std::string msg);
std::string					recv_msg(int sd, bool waiting);
std::string					login(int sd, std::string pw, std::string nick, std::string channel);
bool						targeted(std::string msg, std::string nick);
std::vector<std::string>	getJokes();
void						sendJoke(int sd, std::string channel, std::vector<std::string> jokes);
