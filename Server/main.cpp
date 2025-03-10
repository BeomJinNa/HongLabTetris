#include <iostream>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <boost/locale.hpp>
#include <exception>
#include <set>
#include "Server.h"
#include "Constants.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace
{
void setLocale();
void runServer();
void printSystemInfo();
void setPort(unsigned short& port);
void runThreads(std::vector<std::jthread>& threads,
				boost::asio::io_context& io, Server& server);
}

int main()
{
	try
	{
		setLocale();
		runServer();
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Fatal exception in main: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

namespace
{
void setLocale()
{
	boost::locale::generator gen;
	std::locale loc = gen("UTF-8");
	std::locale::global(loc);
	std::wcin.imbue(loc);
	std::wcout.imbue(loc);

#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
}

void runServer(void)
{
	printSystemInfo();

	unsigned short port = constants::DEFAULT_PORT;
	setPort(port);

	boost::asio::io_context io;
	auto workGuard = boost::asio::make_work_guard(io);

	Server server(io, port, workGuard);

	{
		std::vector<std::jthread> threads;
		runThreads(threads, io, server);
	}

	if (server.IsForcedShutdownRequested() == false)
	{
		std::cout << "Server shutdown gracefully.\n";
	}
}

void printSystemInfo(void)
{
	unsigned int coreCount = std::thread::hardware_concurrency();
	std::cout << "Logical CPU cores: " << coreCount << std::endl;

	std::string hostName = boost::asio::ip::host_name();
	std::cout << "Host Name: " << hostName << std::endl;

	boost::asio::io_context io;
	boost::asio::ip::tcp::resolver resolver(io);
	boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(hostName, "");

	std::set<std::string> ipv4Addresses;
	std::set<std::string> ipv6Addresses;

	for (const auto& entry : endpoints)
	{
		boost::asio::ip::address addr = entry.endpoint().address();
		if (addr.is_v4())
		{
			ipv4Addresses.insert(addr.to_string());
		}
		else if (addr.is_v6())
		{
			ipv6Addresses.insert(addr.to_string());
		}
	}

	std::cout << "IPv4 Addresses:" << std::endl;
	for (const auto& ip : ipv4Addresses)
	{
		std::cout << "  " << ip << std::endl;
	}

	std::cout << "IPv6 Addresses:" << std::endl;
	for (const auto& ip : ipv6Addresses)
	{
		std::cout << "  " << ip << std::endl;
	}
	std::cout << std::endl;
}

void setPort(unsigned short& port)
{
	std::cout << "Enter port number (default " << port << "): ";
	std::string input;
	std::getline(std::cin, input);
	if (!input.empty())
	{
		try
		{
			int p = std::stoi(input);
			if (p > 0 && p <= 65535)
			{
				port = static_cast<unsigned short>(p);
			}
			else
			{
				std::cout << "Invalid port number. Using default " << port << ".\n";
			}
		}
		catch (...)
		{
			std::cout << "Invalid input. Using default port " << port << ".\n";
		}
	}
}

void runThreads(std::vector<std::jthread>& threads, boost::asio::io_context& io, Server& server)
{
	unsigned int threadCount = std::thread::hardware_concurrency();
	if (threadCount == 0)
	{
		threadCount = constants::DEFAULT_THREAD_NUMBER;
	}
	threads.reserve(threadCount);

	auto worker =
		[&io, &server]()
		{
			try
			{
				io.run();
			}
			catch (const std::exception& e)
			{
				std::cerr << "Worker thread exception: " << e.what() << std::endl;
				server.ShutDownServerForced();
			}
		};

	for (unsigned int i = 0; i < threadCount; ++i)
	{
		threads.emplace_back(worker);
	}
	std::cout << "Server started with " << threadCount << " threads.\n";
}
}
