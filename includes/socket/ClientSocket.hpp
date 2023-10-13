/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:16:24 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 01:23:08 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once

#include "socket/ASocket.hpp"

class	ClientSocket: public ASocket
{
	public:
		/* Attributs */

		/* Constructors & Destructors */
		ClientSocket(void);

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

		/* Member functions */

};
