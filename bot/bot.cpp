/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 14:34:09 by hania             #+#    #+#             */
/*   Updated: 2023/11/12 13:04:51 by hania            ###   ########.fr       */
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

#define NB_JOKES 86 // TMP --> Will either count it or save input into a vector

const int			botTimeOut = 1800;
const std::string	delim = "\r\n";
const char			*botInput = "./config/bot.config";

bool	botShutdown = false;

void		signalHandler(int sig) {
	(void)sig;
	botShutdown = true;
}

void		send_msg(int sd, std::string msg) {
	msg += delim;
	int bytes_sent = send(sd, msg.c_str(), msg.size(), 0);
	if (bytes_sent < 0)
		std::cerr << "Unable to send : " << msg << std::endl;
	std::cout << "Sent: " << msg << std::endl;
}

std::string	recv_msg(int sd, bool waiting)
{
	char buffer[4096 + 1];

	struct timeval tv;
	if (waiting == true)
		tv.tv_sec = botTimeOut;
	else
		tv.tv_sec = 2;
	tv.tv_usec = 0;
	setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
	usleep(1000);
	ssize_t bytes_received = recv(sd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received <= 0)
		return "";
	buffer[bytes_received] = '\0';
	return (std::string(buffer));
}

std::string	login(int server_socket, std::string password, std::string nickname, std::string channel)
{
	send_msg(server_socket, "PASS " + password + delim + "NICK " + nickname);
	while (recv_msg(server_socket, 1).find("You are now known as") == std::string::npos) {
		nickname += "_";
		send_msg(server_socket, "NICK " + nickname);
		sleep(1);
	}
	send_msg(server_socket, "USER Bot * * :Mr. Bot" + delim + "JOIN " + channel + delim + "PRIVMSG " + channel + " :Hello! my name is " + nickname + ". Send me a message if you want to hear a programming joke :)");
	return (nickname);
}

void		sendJoke(int sd, std::string channel) {
	std::string		line;
	int				line_nb = 0;
	int				pause = 0;
	std::ifstream	botFile(botInput);

	if (botFile.fail()) {
		std::cerr << "Error: Unable to open " << botInput << std::endl;
		return;
	}
	srand(time(NULL));
	line_nb = std::rand() % NB_JOKES + 1;
	for (int i = 0; i <= line_nb; i++) {
		getline(botFile, line);
	}
	pause = line.find("... ");
	send_msg(sd, "PRIVMSG " + channel + " :" + line.substr(0, pause + 3));
	sleep(3);
	send_msg(sd, "PRIVMSG " + channel + " :" + line.substr(pause, line.length()));
}

int			main(int ac, char **av)
{
	if (ac < 4) {
		std::cerr << "Invalid input. Please try ./bot <server_port> <password> <channel> [nickname] " << std::endl;
		return 1;
	}
	signal(SIGINT, signalHandler);
	std::ifstream	botFile(botInput);
	if (botFile.fail()) {
		std::cerr << "Error: Unable to open " << botInput << std::endl;
		return 1;
	}
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		std::cerr << "Error creating socket." << std::endl;
		close(server_socket);
		return 1;
	}
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(std::atoi(av[1]));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		std::cerr << "Error connecting to the server: " << errno << std::endl;
		close(server_socket);
		return 1;
	}

	std::string	password = static_cast<std::string>(av[2]);
	std::string	channel = (av[3][0] == '#') ? static_cast<std::string>(av[3]) : "#" + static_cast<std::string>(av[3]);
	std::string	nickname = (ac == 5) ? static_cast<std::string>(av[4]) : "bot";

	nickname = login(server_socket, password, nickname, channel);
	while (!botShutdown) {
		std::string	msg = recv_msg(server_socket, 1);
		if (msg.find("PRIVMSG " + nickname) != std::string::npos || msg.find("@" + nickname) != std::string::npos) {
			std::cout << "Recieved: " << msg << std::endl;
			sendJoke(server_socket, channel);
		}
	}
	close(server_socket);
}
