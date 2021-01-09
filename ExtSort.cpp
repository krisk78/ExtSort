// ExtSort.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>

#include "ExtSortApp.hpp"

int main(int argc, char* argv[])
{
	ExtSortApp app{ false };
	auto ret = app.Arguments(argc, argv);
	if (ret == "?")
		return 0;
	if (ret != "")
		return -1;
	int nbfiles{ 0 };
	try
	{
		nbfiles = app.Run();
	}
	catch (const std::exception& exc)
	{
		//if (app.windows_mode())
		//
		//else
			std::cout << exc.what() << std::endl;
		return -1;
	}
	std::cout << std::endl << nbfiles << " files processed." << std::endl;
	return 0;
}
