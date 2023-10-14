/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:16:24 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 11:40:06 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

// INCLUDES

#include "socket/ASocket.hpp"
#include "socket/ServerSocket.hpp"

// MACROS

#define CLIENTOPTSIZE 6

class	ClientSocket: public ASocket
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		ClientSocket(const ServerSocket &server);

		ClientSocket(const ClientSocket &other);
		ClientSocket	&operator=(const ClientSocket &other);

		virtual ~ClientSocket(void);

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
		ClientSocket(void);

		/* Member functions */
		void	setSocketOptions(void);
};
