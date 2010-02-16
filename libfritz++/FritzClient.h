/*
 * libfritz++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libfritz@joachim-wilke.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef FRITZCLIENT_H_
#define FRITZCLIENT_H_

#include <stdlib.h>
#include <PThread++.h>
#include "Tools.h"
#include <TcpClient++.h>

namespace fritz {

class FritzClient {
private:
	static pthread::Mutex* mutex;
    std::string CalculateLoginResponse(std::string challenge);
	std::string UrlEncode(std::string &s);
	bool Login() throw(tcpclient::TcpException);
	std::string GetLang() throw(tcpclient::TcpException);
	bool validPassword;
public:
	FritzClient ();
	virtual ~FritzClient();
	bool InitCall(std::string &number);
	std::string RequestLocationSettings();
	std::string RequestSipSettings();
	std::string RequestCallList();
	std::string RequestFonbook();
	bool hasValidPassword() { return validPassword; }
};

}

#endif /* FRITZCLIENT_H_ */
