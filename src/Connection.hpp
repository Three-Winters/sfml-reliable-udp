#pragma once

#include <SFML/Network.hpp>

class Connection {
public:
  Connection(sf::IpAddress iaddr, unsigned short int iport);
  ~Connection();
  sf::IpAddress addr;
  unsigned short int port;
  sf::Uint16 seq = 0;
  int32_t ping = 0;
  sf::Time last_time;
  bool wrap_flag = false;
  bool pending = true;
  sf::Uint32 key = 0;
  uint64_t lost_packets = 0;
private:
};
