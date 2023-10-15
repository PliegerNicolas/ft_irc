/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:14:59 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/15 01:07:20 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ASocket.hpp"

// MACROS

#define SERVOPTSIZE 6

class	ServerSocket: public ASocket
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		ServerSocket(void);
		ServerSocket(const t_soconfig &socketConfig);
		//ServerSocket(const int &domain, const int &service, const int &protocol,
		//	const std::string &interface, const int &port);

		ServerSocket(const ServerSocket &other);
		ServerSocket	&operator=(const ServerSocket &other);

		virtual ~ServerSocket(void);

		/* Member functions */

		// Getter

		// Setter

	protected:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */

	private:
		/* Attributs */

		/* Constructors & Destructors */

		/* Member functions */
		void	setSocketOptions(void);

		int		bindToNetwork(void);
		int		listenToNetwork(void);
};
