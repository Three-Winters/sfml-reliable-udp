#include "UdpServerBase.hpp"
#include <iostream>
#include <cstdint>

void UdpServerBase::process2(sf::Packet p, sf::IpAddress sender, unsigned short port) {
  sf::Uint16 seq_num;
  std::cout<<"sender: "<<sender<<" port: "<<port<<std::endl;
  std::shared_ptr<Connection> conn = find_connection(sender, port);
  
  list_all_connections();
  std::cout<<"Got: "<<sender<<" "<<port<<std::endl;
  
  if(conn) {
	std::cout<<" ping: "<<conn->ping<<std::endl;
	p >> seq_num;
	flag_connection_seq_wrap(conn);
	if(compare_seqs(conn, seq_num) == true) {
	  conn->seq = seq_num;
	  conn->last_time = server_clock.getElapsedTime();
	} 
	std::cout<<"seq_num: "<<conn->seq<<std::endl;
  }
}

void UdpServerBase::get_pings() {
  //This function iterates over connection vector, calculates the pings and looks for timeouts.
  for (auto i = connections.begin() ; i!=connections.end() ;) {
	if((*i)) {
	  (*i)->ping = abs(ping_time((*i)->last_time));
	  if(check_for_timeout((*i)->ping)) {
		unregister_connection((*i));
	  } else {
		i++;
	  }
	}
  }
}

bool UdpServerBase::check_for_timeout(int32_t ping) {
  //return true if ping is too high
  if (ping>PING_TIMEOUT) {
	return(true);
  } else {
	return(false);
  }
}

UdpServerBase::UdpServerBase() {
  if(socket.bind(bind_port) != sf::Socket::Done) {
	std::cout << "Failed to bind server to port " << bind_port << std::endl;
	abort();
  }
  connections.reserve(max_connections);
}
UdpServerBase::~UdpServerBase() {}

sf::UdpSocket UdpServerBase::socket;

void UdpServerBase::poll() {
  sf::Packet packet;
  sf::IpAddress sender;
  unsigned short port;
  get_pings();
  
  socket.setBlocking(false);
  sf::Socket::Status status = socket.receive(packet, sender, port);
  if(status == sf::Socket::Status::Done) {
	mode_sort(packet, sender, port);
  }
}

void UdpServerBase::mode_sort(sf::Packet p, sf::IpAddress sender, unsigned short port) {
  sf::Uint8 mode;
  p >> mode;
  if(mode == Netcodes::SYN) {
	//connect
	handle_connection(p, sender, port);
	std::cout<<"c_mode"<<std::endl;
  } else if (mode == Netcodes::SYN_ACK) {
	associate_connection(p, sender, port);
	std::cout<<"a_mode"<<std::endl;
  } else if (mode > 1) {
	if(process) {
	  process(p, sender, port);
	  //std::cout<<"u_mode: "<<mode<<std::endl;
	}
  }
}
int UdpServerBase::num_connections(sf::IpAddress sender) {
  //gets the total number of connections from an IpAddress
  int num_cons = 0;
  for (auto i=connections.begin() ; i!=connections.end() ; i++) {
	if((*i)) {
	  if((*i)->addr == sender) {
		num_cons++;
	  }
	}
  }
  return(num_cons);
}

bool UdpServerBase::check_num_connections(int num_conns) {
  //A function to check that the number of connections per IP doesn't exceed max_con_connections
  if(num_conns>max_con_connections) {
	return (false);
  } else {
	return(true);
  }
}

void UdpServerBase::handle_connection(sf::Packet p, sf::IpAddress sender, unsigned short port) {
  if(check_num_connections(num_connections(sender))) {
	std::minstd_rand0 g1(random_device());
	
	sf::Uint8 x = 1;
	sf::Uint16 use_port = g1();
	sf::Uint32 key = g1();
	sf::Packet ack;

	//ports in range 1-1023 are privledged
	while(use_port<1023) {
	  use_port = g1();
	}
	
	ack << x << key << use_port;
	
	socket.send(ack, sender, port);

	if(connections.size() < max_connections) {
	  if(find_connection(sender, use_port) == nullptr) {
		std::shared_ptr<Connection> pending (new Connection(sender, use_port));
		pending->last_time = server_clock.getElapsedTime();
		connections.push_back(std::move(pending));
	  }
	} else {
	  //too many connections...
	}
  }
}


void UdpServerBase::associate_connection(sf::Packet p, sf::IpAddress sender, unsigned short port) {
  //complete the handshake here and let the connection be valid
  std::shared_ptr<Connection> conn = find_connection(sender, port);
  if(conn) {
	sf::Uint32 this_key;
	p >> this_key;
	if (this_key == conn->key) {
	  if(conn->pending == true) {
		conn->pending = false;
	  }
	}
  }
}

void UdpServerBase::unregister_connection(sf::IpAddress sender, unsigned short port, std::vector<std::shared_ptr<Connection>> &database) {
  for (auto i=database.begin() ; i!=database.end();) {
	if((*i)) {
	  sf::IpAddress seen_addr = (*i)->addr;
	  unsigned short int seen_port = (*i)->port;
	  if (sender == seen_addr && port == seen_port) {
		i = database.erase(i);
	  } else {
		i++;
	  }
	} else {
	  i++;
	}
  }
}

void UdpServerBase::unregister_connection(std::shared_ptr<Connection> conn) {
  if(conn) {
	sf::IpAddress sender = conn->addr;
	unsigned short int port = conn->port;
  
	for (auto i=connections.begin() ; i!=connections.end() ;) {
	  if((*i)) {
		sf::IpAddress seen_addr = (*i)->addr;
		unsigned short int seen_port = (*i)->port;
		if(seen_addr == sender && seen_port == port) {
		  i = connections.erase(i);
		} else {
		  i++;
		}
	  } else {
		i++;
	  }
	}
  }
}

std::shared_ptr<Connection> UdpServerBase::find_connection(sf::IpAddress sender, unsigned short port) {
  for (auto i=connections.begin() ; i!=connections.end() ; i++) {
	if((*i)) {
	  if((*i)->addr == sender && (*i)->port == port) {
		return((*i));
	  }
	}
  }
  return(nullptr);
}

void UdpServerBase::list_all_connections() {
  //a function to just print out the information of all connected clients
  for (auto i=connections.begin() ; i!=connections.end() ; i++) {
	if((*i)) {
	  std::cout<<"ADDR: "<<(*i)->addr<<" PORT: "<<(*i)->port<<std::endl;
	} else {
	  std::cout<<"EMPTY"<<std::endl;
	}
  }
}

void UdpServerBase::flag_connection_seq_wrap(std::shared_ptr<Connection> conn) {
  //Here we look if sequence numbers are in the upper fiths of their max value to predict integer overflow
  if(conn->seq > (UINT16_MAX - FITH)) {
	conn->wrap_flag = true;
  }
}

bool UdpServerBase::compare_seqs(std::shared_ptr<Connection> conn, sf::Uint16 n_seq) {
  //we return false if new sequence is older than connection sequence so we know to discard it.
  if(conn->seq > n_seq) {
	return(check_wrap(conn, n_seq));
  } else {//(conn->seq <= n_seq)
	if(n_seq && conn->seq) {
	  conn->lost_packets += (n_seq) - (conn->seq+1);
	}
	flag_connection_seq_wrap(conn);
	return(true);
  }
}

bool UdpServerBase::check_wrap(std::shared_ptr<Connection> conn, sf::Uint16 n_seq) {
  //Here we see if we've finally wrapped around.
  if(conn->wrap_flag == true && n_seq < FITH) {
	conn->seq = 0;
	conn->wrap_flag = false;
	return(true);
  } else {
	conn->lost_packets++;
	return(false);
  }
}

int32_t UdpServerBase::ping_time(sf::Time last) {
  //calculates the time inbetween packets
  return(last.asMilliseconds() - server_clock.getElapsedTime().asMilliseconds());
}

void UdpServerBase::set_port(unsigned short int port) {
  bind_port = port;
}

