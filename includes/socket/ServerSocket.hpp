/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:14:59 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 01:59:30 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ASocket.hpp"

// MACROS

#define SERVOPTSIZE 6
#define SERVBACKLOG 15

class	ServerSocket: public ASocket
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		ServerSocket(void);

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
