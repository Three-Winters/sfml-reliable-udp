#include <thread>
#include <chrono>
#include <iostream>
#include "UdpClientBase.hpp"

sf::UdpSocket UdpClientBase::socket;

void UdpClientBase::process(sf::Packet p, sf::IpAddress sender, unsigned short port) {
  sf::Uint8 mode;
  p >> mode;
  
  switch(mode) {
	  case Netcodes::SYN_ACK:
		if(pending) {
		  associate(p, sender);
		  std::cout<<"got ack\n";
		}
		break;
	  default:
		std::cout<<"unhandled code: "<<mode<<std::endl;
		break;
  }	
}

UdpClientBase::UdpClientBase() {
  socket.setBlocking(false);
}

UdpClientBase::~UdpClientBase() {}

void UdpClientBase::poll() {
  sf::Packet packet;
  sf::IpAddress sender;
  unsigned short int c_port = CONN_PORT;

  sf::Socket::Status status = socket.receive(packet, sender, c_port);
  if(status == sf::Socket::Status::Done) {
	process(packet, sender, CONN_PORT);
  }
  if(!pending) {
	sf::Packet seq_p;
	seq_p << (sf::Uint8)10;
	seq_p << seq;
	
	socket.send(seq_p, sf::IpAddress::LocalHost, CONN_PORT);
	seq++;
	//seq_num += 4;
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}


int32_t UdpClientBase::ping_time(sf::Time last) {
  //calculates the time inbetween packets
  return(last.asMilliseconds() - client_clock.getElapsedTime().asMilliseconds());
}

void UdpClientBase::set_port(unsigned short int a_port) {
  port = a_port;
}

bool UdpClientBase::connect(sf::IpAddress addr, unsigned short int port) {
  if(socket.bind(port) != sf::Socket::Done) {
	return(false);
  } else {
	sf::Uint8 conn_code = Netcodes::SYN;
	sf::Packet connect1;
	connect1 << conn_code;
	socket.send(connect1, addr, CONN_PORT);
	std::cout<<"port: "<<port<<std::endl;
	return(true);
  }
}

void UdpClientBase::associate(sf::Packet p, sf::IpAddress server) {
  sf::Uint16 p_use_port;
  sf::Uint32 p_key;
  p >> p_key >> p_use_port;
  key = p_key;
  port = p_use_port;

  sf::Packet connect2;
  sf::Uint8 code = Netcodes::SYN_ACK;
  connect2 << code << key;
  socket.unbind();
  
  if(connect(server, port)) {
	
	socket.send(connect2, server, CONN_PORT);
	pending = false;
  }
}

bool UdpClientBase::disconnect() {
  socket.unbind();
  return(true);
}

void UdpClientBase::flag_wrap() {
  if(server_seq > (UINT16_MAX - FITH)) {
	server_wrap_flag = true;
  }
}

bool UdpClientBase::compare_seqs(sf::Uint16 n_seq) {
  if(server_seq > n_seq) {
	return(check_wrap(n_seq));
  } else {
	lost_packets += n_seq - (server_seq + 1);
	return(true);
  }
}

bool UdpClientBase::check_wrap(sf::Uint16 n_seq) {
  if(server_wrap_flag == true && n_seq < FITH) {
	server_seq = 0;
	server_wrap_flag = false;
	return(true);
  } else {
	lost_packets++;
	return(false);
  }
}

void UdpClientBase::ackable(sf::Packet p) {
  ack_q.push(p);
}

void UdpClientBase::resend() {
  //this is a copy of the packet on top of the stack.
  sf::Packet p = ack_q.top();
  p<<seq;
  seq++;
  socket.send(p, server_addr, port);
}
