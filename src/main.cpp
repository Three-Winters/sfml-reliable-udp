#include <iostream>
#include <cstring>
#include <SFML/Network.hpp>
#include "UdpServerBase.hpp"
#include "UdpClientBase.hpp"

int main(int argc, char** argv) {
  /*
  sf::Uint8 x = 10;
  std::string s = "hello";
  double d = 0.6;

  sf::Packet packet;
  packet << x << s << d;
  sf::UdpSocket	socket;
  if(socket.bind(54000) != sf::Socket::Done) {
	//error
  }
  socket.send(packet, sf::IpAddress::LocalHost, 54001);
  */
  char s[] = "server";
  if(argc == 2 && (strcmp(argv[1], s)) == 0) {
	UdpServerBase* my_serv = new UdpServerBase();
 
	ProcessPacketsFun pl = [my_serv](sf::Packet p, sf::IpAddress sender, unsigned short port) {
							 sf::Uint16 seq_num;
							 std::cout<<"sender: "<<sender<<" port: "<<port<<std::endl;
							 std::shared_ptr<Connection> conn = my_serv->find_connection(sender, port);
  
							 my_serv->list_all_connections();
							 std::cout<<"Got: "<<sender<<" "<<port<<std::endl;
  
							 if(conn) {
							   std::cout<<" ping: "<<conn->ping<<std::endl;
							   p >> seq_num;
							   my_serv->flag_connection_seq_wrap(conn);
							   if(my_serv->compare_seqs(conn, seq_num) == true) {
								 conn->seq = seq_num;
								 conn->last_time = my_serv->server_clock.getElapsedTime();
							   } 
							   std::cout<<"seq_num: "<<conn->seq<<std::endl;
							   std::cout<<"Lost: "<<conn->lost_packets<<std::endl;
							 }
						   };
	my_serv->process = pl;
  
	while(true) {
	  my_serv->poll();
	}
  } else {
	UdpClientBase* my_client = new UdpClientBase();
	my_client->connect(sf::IpAddress::LocalHost, 54000);
	while(true) {
	  my_client->poll();
	}
  }
  return(0);
}
