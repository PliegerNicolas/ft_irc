/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 14:34:09 by hania             #+#    #+#             */
/*   Updated: 2023/11/09 20:56:30 by hania            ###   ########.fr       */
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

void	send_msg(int sd, std::string msg) {
	msg += "\r\n";
	int bytes_sent = send(sd, msg.c_str(), msg.size(), 0);
	if (bytes_sent < 0)
		std::cerr << "Unable to send : " << msg << std::endl;
	std::cout << "Sent: " << msg << std::endl;
}

// ./bot <server_port> <password> <nickname> <channel>
int	main(int ac, char **av)
{
	if (ac != 5)
		return (0);
    int irc_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6667);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    connect(irc_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    send_msg(irc_socket, "CAP LS");
    send_msg(irc_socket, "PASS " + (std::string)av[2]);
    send_msg(irc_socket, "NICK " + (std::string)av[3]);
    send_msg(irc_socket, "USER Bot * * :Mr Bot");
    send_msg(irc_socket, "JOIN #" + (std::string)av[4]);
	while (1)
	{
		std::ifstream	botFile("./input.config");
		std::string		line;
		int				line_nb;

		if (!botFile.is_open())
			std::cerr << "Unable to open the bot's input file" << std::endl;
		srand(time(NULL));
		line_nb = std::rand() % 31 + 1;
		for (int i = 0; i <= line_nb; i++)
		{
			getline(botFile, line);
		}
		send_msg(irc_socket, "PRIVMSG #h : " + line);
		sleep(20);
	}
}
