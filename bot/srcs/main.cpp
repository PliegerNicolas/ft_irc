/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/12 13:41:33 by hania             #+#    #+#             */
/*   Updated: 2023/11/12 17:26:08 by hania            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/bot.hpp"

bool	botShutdown = false;

void		signalHandler(int sig) {
	(void)sig;
	botShutdown = true;
}

int			main(int ac, char **av)
{
	std::vector<std::string>	jokes;

	if (ac < 4) {
		std::cerr << "Invalid input. Please try ./ircbot <server_port> <password> <channel> [nickname] " << std::endl;
		return 1;
	}
	signal(SIGINT, signalHandler);

	jokes = getJokes();
	if (jokes.empty())
		return 1;
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
		if (msg.empty()) {
			std::cout << "Disconnected (Remote host closed socket)" << std::endl;
			close(server_socket);
			return 1;
		}
		if (targeted(msg, nickname)) {
			std::cout << "Recieved: " << msg << std::endl;
			sendJoke(server_socket, channel, jokes);
		}
	}
	close(server_socket);
	return 0;
}
