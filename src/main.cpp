#include <iostream>
#include <cstring>
#include <SFML/Network.hpp>
#include <thread>
#include <mutex>
#include "UdpServerBase.hpp"
#include "UdpClientBase.hpp"

std::mutex m;
char c;
std::string str;

void console() {
  m.lock();
  std::cin >> str;
  m.unlock();
}

std::thread make_input_thread() {
  std::thread input_thread (console);
  return(input_thread);
}

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

	make_input_thread().detach();
  
	while(true) {
	  //my_serv->poll();
	  std::thread poll_thread (&UdpServerBase::poll, my_serv);

	  if (str != "") {
		m.lock();
		std::cout << "You typed: " << str << std::endl;
		str = "";
		m.unlock();
		make_input_thread().detach();
	  }
	  poll_thread.join();
	}
  } else {
	UdpClientBase* my_client = new UdpClientBase();
	my_client->connect(sf::IpAddress::LocalHost, 54000);
	
	make_input_thread().detach();
	
	while(true) {
	  std::thread poll_thread (&UdpClientBase::poll, my_client);

	  if (str != "") {
		m.lock();
		std::cout << "You typed: " << str << std::endl;
		str = "";
		m.unlock();
		make_input_thread().detach();
	  }
	  poll_thread.join();
	}
  }
  return(0);
}
