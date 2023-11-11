/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 14:34:09 by hania             #+#    #+#             */
/*   Updated: 2023/11/10 21:55:27 by hania            ###   ########.fr       */
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

bool	bot_end = false;

void	signalHandler(int sig) {
	(void)sig;
	bot_end = true;
}

void	send_msg(int sd, std::string msg) {
	msg += "\r\n";
	int bytes_sent = send(sd, msg.c_str(), msg.size(), 0);
	if (bytes_sent < 0)
		std::cerr << "Unable to send : " << msg << std::endl;
	std::cout << "Sent: " << msg << std::endl;
}

std::string	recv_msg(int sd, bool registered)
{
	char buffer[4096 + 1];

	struct timeval tv;
	if (registered == true)
		tv.tv_sec = 5;
	else
		tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
	usleep(1000);
	ssize_t bytes_received = recv(sd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received <= 0) {
		bot_end = true;
		return "";
	}
	buffer[bytes_received] = '\0';
	return (std::string(buffer));
}

int	main(int ac, char **av)
{
	if (ac != 5) {
		std::cerr << "Invalid input. Please try ./bot <server_port> <password> <nickname> <channel>" << std::endl;
		return 1;
	}
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		std::cerr << "Error creating socket." << std::endl;
		return 1;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(6667);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr) == -1)) {
		std::cerr << "Error connecting to the server." << std::endl;
		close(server_socket);
		return 1;
	}
	std::string	nickname = (std::string)av[3];
	std::string	channel = (std::string)av[4];

	send_msg(server_socket, "PASS " + (std::string)av[2]);
	send_msg(server_socket, "NICK " + nickname);
	while (!recv_msg(server_socket, 0).find("You are now known as")) {
		nickname += "_";
		send_msg(server_socket, "NICK " + nickname);
		sleep(1);
	}
	send_msg(server_socket, "USER Bot * * :Mr Bot");
	recv_msg(server_socket, 1);
	send_msg(server_socket, "JOIN #" + channel);
	send_msg(server_socket, "PRIVMSG #" + channel + " :Hello! my name is " + nickname + ". Send me a message if you want to hear a programming joke :)");
	recv_msg(server_socket, 1);

	signal(SIGINT, signalHandler);
	while (!bot_end)
	{
		if (!recv_msg(server_socket, 1).find("localhost"))
		{
			std::ifstream	botFile("./config/bot.config");
			std::string		line;
			int				line_nb;
			int				pause;

			if (!botFile.is_open())
			{
				std::cerr << "Unable to open the bot's input file" << std::endl;
				return (0);
			}
			srand(time(NULL));
			line_nb = std::rand() % 48 + 1;
			for (int i = 0; i <= line_nb; i++)
			{
				getline(botFile, line);
			}
			pause = line.find("... ");
			send_msg(server_socket, "PRIVMSG #" + channel + " :" + line.substr(0, pause + 3));
			sleep(7);
			send_msg(server_socket, "PRIVMSG #" + channel + " :" + line.substr(pause, line.length()));
		}
	}
}
