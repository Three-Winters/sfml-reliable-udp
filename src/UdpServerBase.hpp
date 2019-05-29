#pragma once

#include <SFML/Network.hpp>

#include <random>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <thread>

#include "Connection.hpp"

typedef std::function<void (sf::Packet p, sf::IpAddress sender, unsigned short port)> ProcessPacketsFun;

class UdpServerBase {
public:
  enum Netcodes : sf::Uint8 {
	SYN,
	SYN_ACK
  };
  
  const uint16_t FITH = 0x3333; //A fith of the max value of uint16_t
  unsigned long int max_connections = 24;
  int32_t PING_TIMEOUT = 65536;
  uint32_t poll_rate = 0;
  uint8_t max_con_connections = 8;
  ProcessPacketsFun process = NULL;

  void get_pings();
  bool check_for_timeout(int32_t ping);
  
  UdpServerBase();
  ~UdpServerBase();
  void poll();
  void set_port(unsigned short int port);

  sf::Clock server_clock;
  //std::vector<std::shared_ptr<Connection>> connections = {};
  std::vector<std::shared_ptr<Connection>> connections;
  
  std::shared_ptr<Connection> find_connection(sf::IpAddress sender, unsigned short port);
  void flag_connection_seq_wrap(std::shared_ptr<Connection> conn);
  bool compare_seqs(std::shared_ptr<Connection> conn, sf::Uint16 n_seq);
  bool check_wrap(std::shared_ptr<Connection> conn, sf::Uint16 n_seq);

  void list_all_connections();

  void process2(sf::Packet p, sf::IpAddress sender, unsigned short port);
  
private:
  std::random_device random_device;
  
  std::vector<std::shared_ptr<Connection>> pending_connections;

  void mode_sort(sf::Packet p, sf::IpAddress sender, unsigned short port);
  int num_connections(sf::IpAddress sender);
  bool check_num_connections(int num_conns);
  void handle_connection(sf::Packet p, sf::IpAddress sender, unsigned short port);
  void associate_connection(sf::Packet p, sf::IpAddress sender, unsigned short port);
  void unregister_connection(sf::IpAddress sender, unsigned short port, std::vector<std::shared_ptr<Connection>> &database);
  //void unregister_connection(Connection* conn);
  void unregister_connection(std::shared_ptr<Connection> conn);
  
  int32_t ping_time(sf::Time last);

  static sf::UdpSocket socket;
  unsigned short int bind_port = 54001;
};
