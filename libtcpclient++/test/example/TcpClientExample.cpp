/*
 * TcpClientExample.cpp
 *
 *  Created on: 25.08.2009
 *      Author: wilke
 */

#include <TcpClient++.h>
#include <iostream>

// uncomment one of the tests
//#define TEST_TCP_WWW
//#define TEST_HTTP_WWW
//#define TEST_TCP_LOCAL
//#define TEST_TCP_LOCAL_LOOP
#define TEST_HTTP_LOCAL

int main() {
#ifdef TEST_TCP_WWW
	tcpclient::TcpClient hc("www.mx2.eu", 80);
#endif
#ifdef TEST_HTTP_WWW
	tcpclient::HttpClient hc("www.mx2.eu", 80);
#endif
#if defined(TEST_TCP_LOCAL) || defined(TEST_TCP_LOCAL_LOOP)
	tcpclient::TcpClient hc("localhost", 3333);
#endif
#ifdef TEST_HTTP_LOCAL
	tcpclient::HttpClient hc("localhost", 3333);
#endif
#if defined(TEST_HTTP_WWW) || defined (TEST_HTTP_LOCAL)
	//hc << "Test." << std::endl;
	//hc << tcpclient::post  << "/index.htm" << std::flush << "payload" << std::flush;
	//hc << tcpclient::get << "/index.htm" << std::flush;
	//hc << tcpclient::post  << "/index.htm" << "\nX-Header: Bla" << std::flush << "payload" << std::flush;
	hc << tcpclient::get << "/index.htm"  << "\nX-Header: Bla\n" << std::flush;
#else
	hc << "Hello world!" << std::flush;
#endif
	std::string tmp;
#ifdef TEST_TCP_LOCAL_LOOP
	do {
#endif
	hc >> tmp;
	std::cout << tmp << "_";
#ifdef TEST_TCP_LOCAL_LOOP
	} while (true);
#endif
	return 0;
}
