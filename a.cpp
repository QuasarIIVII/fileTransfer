#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>

int main(int argc, char *argv[]) {
    std::string mode;
    std::vector<std::string> requiredInputs;
    std::string port_number;
    bool hasOptionalInput = false;

    if (argc >= 2) {
        mode = std::string(argv[1]);
    }
    else {
        std::cerr << "Missing mode argument." << std::endl;
        return 1;
    }

    if (mode != "send" && mode != "receive") {
        std::cerr << "Invalid mode argument." << std::endl;
        return 1;
    }

    if (mode == "send") {
        if (argc < 4) {
            std::cerr << "Missing required input(s)." << std::endl;
            return 1;
        }

        for (int i = 2; i < argc; ++i) {
            if (std::string(argv[i]) == "-P") {
                if (i == argc - 1) {
                    std::cerr << "Missing optional input value." << std::endl;
                    return 1;
                }
                port_number = std::string(argv[i+1]);
                hasOptionalInput = true;
                ++i;
            }
            else {
                requiredInputs.push_back(std::string(argv[i]));
            }
        }

        if (requiredInputs.size() != 2) {
            std::cerr << "Invalid number of required inputs." << std::endl;
            return 1;
        }
    }
    else {  // mode == "receive"
        if (argc < 3) {
            std::cerr << "Missing required input(s)." << std::endl;
            return 1;
        }

        for (int i = 2; i < argc; ++i) {
            if (std::string(argv[i]) == "-P") {
                if (i == argc - 1) {
                    std::cerr << "Missing optional input value." << std::endl;
                    return 1;
                }
                port_number = std::string(argv[i+1]);
                hasOptionalInput = true;
                ++i;
            }
            else {
                requiredInputs.push_back(std::string(argv[i]));
            }
        }

        if (requiredInputs.size() != 1) {
            std::cerr << "Invalid number of required inputs." << std::endl;
            return 1;
        }

    }

    std::cout << "Mode: " << mode << std::endl;


    if (mode == "send") {
        std::string file_name = requiredInputs[0];
        std::string ip_address = requiredInputs[1];
		if(!hasOptionalInput)port_number="57001";

        std::cout << "File : " << file_name << std::endl;
        std::cout << "IP Address: " << ip_address << std::endl;
        std::cout << "Port: " << port_number << std::endl;

		// Open the file
		std::ifstream file(file_name, std::ios::binary);
		if (!file) {
			std::cerr << "Error: cannot open file " << file_name << std::endl;
			return 1;
		}

		// Create a socket
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1) {
			std::cerr << "Error: cannot create socket" << std::endl;
			return 1;
		}

		// Set the IP address and port of the server
		struct sockaddr_in server_address;
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = inet_addr(ip_address.c_str());
		server_address.sin_port = htons(std::stoi(port_number));

		std::cout << "Connecting to " << inet_ntoa(server_address.sin_addr) 
				  << " on port " << ntohs(server_address.sin_port) << std::endl;

		// Connect to the server
		if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
			std::cerr << "Error: cannot connect to " << ip_address << ":" << port_number << std::endl;
			return 1;
		}

		// Send the file data
		char buffer[1024];
		while (file.read(buffer, sizeof(buffer)), file.gcount()) {
			if (send(sock, buffer, file.gcount(), 0) == -1) {
				std::cerr << "Error: failed to send data" << std::endl;
				return 1;
			}
		}

		// Close the socket and the file
		close(sock);
		file.close();

		std::cout << "File sent successfully" << std::endl;
    }
	else{
        std::string file_name = requiredInputs[0];
		if(!hasOptionalInput)port_number="57001";

        std::cout << "File : " << file_name << std::endl;
        std::cout << "Port: " << port_number << std::endl;

		int server_fd, new_socket;
		struct sockaddr_in address;
		int opt = 1;
		int addrlen = sizeof(address);

		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
			std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
			return 1;
		}

		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
			std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
			return 1;
		}

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(std::stoi(port_number));

		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
			std::cerr << "Bind failed: " << strerror(errno) << std::endl;
			return 1;
		}

		if (listen(server_fd, 3) < 0) {
			std::cerr << "Listen failed: " << strerror(errno) << std::endl;
			return 1;
		}

		std::cout << "Waiting for connection..." << std::endl;

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
//		socklen_t scklen=sizeof(address);
//		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &scklen)) < 0) {
			std::cerr << "Accept failed: " << strerror(errno) << std::endl;
			return 1;
		}

		std::cout << "Connection established" << std::endl;

		std::ofstream output_file(file_name, std::ios::binary);
		if (!output_file) {
			std::cerr << "Failed to open file for writing" << std::endl;
			return 1;
		}

		const int buffer_size = 1024;
		char buffer[buffer_size];
		int bytes_read;
		while ((bytes_read = read(new_socket, buffer, buffer_size)) > 0) {
			output_file.write(buffer, bytes_read);
		}
		if (bytes_read < 0) {
			std::cerr << "Read error: " << strerror(errno) << std::endl;
			return 1;
		}

		std::cout << "File received" << std::endl;

		close(new_socket);
		close(server_fd);
	}


    return 0;
}

