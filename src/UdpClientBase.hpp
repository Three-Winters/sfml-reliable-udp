#pragma once

#include <stack>
#include <SFML/Network.hpp>

class UdpClientBase {
public:
  enum Netcodes : sf::Uint8 {
	SYN,
	SYN_ACK
  };
  
  UdpClientBase();
  ~UdpClientBase();

  const unsigned short int CONN_PORT = 54001;
  const uint16_t FITH = 0x3333;
  
  unsigned short int port = 54000;
  sf::Uint16 seq = 0;
  sf::Uint16 server_seq = 0;
  sf::IpAddress server_addr;
  bool server_wrap_flag = false;
  int32_t PING_TIMEOUT = 65536;
  int32_t ping = 0;
  sf::Time last_time;
  bool pending = true;
  sf::Uint32 key = 0;
  uint64_t lost_packets = 0;
  uint64_t total_sent = 0;
  sf::Clock client_clock;

  std::stack<sf::Packet> ack_q;

  void poll();
  int32_t ping_time(sf::Time last);
  void set_port(unsigned short int port);
  bool connect(sf::IpAddress addr, unsigned short int port);
  void associate(sf::Packet p, sf::IpAddress server);
  bool disconnect();

  void flag_wrap();
  bool compare_seqs(sf::Uint16 n_seq);
  bool check_wrap(sf::Uint16 n_seq);

  void ackable(sf::Packet p);
  void resend();

  static sf::UdpSocket socket;
private:
  void process(sf::Packet p, sf::IpAddress sender, unsigned short port);
};
