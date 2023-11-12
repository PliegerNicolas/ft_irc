/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 14:34:09 by hania             #+#    #+#             */
/*   Updated: 2023/11/12 15:19:38 by hania            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/bot.hpp"

const int			botTimeOut = 1800;
const std::string	delim = "\r\n";
const char			*botInput = "./config/bot.config";

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

std::string	login(int sd, std::string pw, std::string nick, std::string channel)
{
	send_msg(sd, "PASS " + pw + delim + "NICK " + nick);
	while (recv_msg(sd, 1).find("You are now known as") == std::string::npos) {
		nick += "_";
		send_msg(sd, "NICK " + nick);
		sleep(1);
	}
	send_msg(sd, "USER Bot * * :Mr. Bot" + delim + "JOIN " + channel + delim +
		"PRIVMSG " + channel + " :Hello! my name is " + nick +
		". Send me a message if you want to hear a programming joke :)");
	return (nick);
}

bool		targeted(std::string msg, std::string nick) {
	return (msg.find("PRIVMSG " + nick + "\n") != std::string::npos
		|| msg.find("PRIVMSG " + nick + " ") != std::string::npos
		|| msg.find("PRIVMSG " + nick + delim) != std::string::npos
		|| msg.find("@" + nick + "\n") != std::string::npos
		|| msg.find("@" + nick + " ") != std::string::npos
		|| msg.find("@" + nick + delim) != std::string::npos);
}

std::vector<std::string>	getJokes() {
	std::ifstream				botFile(botInput);
	std::string					line;
	std::vector<std::string>	jokes;

	if (botFile.fail()) {
		std::cerr << "Error: Unable to open " << botInput << std::endl;
		return (jokes);
	}
	while (getline(botFile, line)) {
		jokes.push_back(line);
	}
	return (jokes);
}

void		sendJoke(int sd, std::string channel, std::vector<std::string> jokes)
{
	int				line_nb = 0;
	int				pause = 0;
	std::string		joke;

	srand(time(NULL));
	line_nb = std::rand() % jokes.size() + 1;
	joke = jokes[line_nb];
	pause = joke.find("... ");
	send_msg(sd, "PRIVMSG " + channel + " :" + joke.substr(0, pause + 3));
	sleep(3);
	send_msg(sd, "PRIVMSG " + channel + " :" + joke.substr(pause, joke.length()));
}
