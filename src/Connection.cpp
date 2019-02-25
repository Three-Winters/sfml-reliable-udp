#include "Connection.hpp"

Connection::Connection(sf::IpAddress iaddr, unsigned short int iport) {
  addr = iaddr;
  port = iport;
}

Connection::~Connection() {}
