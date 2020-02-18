#include <fstream>
#include <string>
#include <iostream>
#include <list>
#include <unordered_map>
#include <algorithm>

#include "library.hpp"

struct Page
{
	std::string name;
	std::list <std::string> links;
	std::list <std::string> breakTos;
};

using Chapter = std::unordered_map <std::string, Page>;

static Chapter gData;

// mayhap just write a dotfile for sfdp
int main (int argc, char ** argv)
{
	Page start;

	if (argc <= 1)
	{
		std::cerr << "Error! Need input file" << std::endl;
		return 1;
	}

	// just create data structures
	for (int i = 1; i < argc; ++i)
	{
		std::ifstream reading (argv [i]);
		if (!reading.is_open())
		{
			std::cerr << "Warning! Couldn't open file " << argv [i] << ", continuing" << std::endl;
			continue;
		}


		std::string line;
		Page * writingTo {nullptr};
		while (std::getline (reading, line))
		{
			if (line.length() == 0)
				continue;

			line = trim_whitespace (line);
			switch (line [0])
			{
			case '#':
				{
					const auto name {to_lower (make_safe (line.substr (1)))};
					writingTo = &gData [name];
					writingTo->name = name;
					break;
				}
			case '~':
				if (writingTo != nullptr)
				{
					const auto p {line.find (':')};
					if (p == std::string::npos or p+2 >= line.length())
						break;

					const auto fn {to_lower (line.substr (1, p-1))};
					const auto ft {to_lower (make_safe (line.substr (p+1)))};

					if (fn == "link" or fn == "tag" or fn == "linkto")
					{
						writingTo->links.push_front (ft);
					}
					else if (fn == "break")
					{
						writingTo->breakTos.push_front (ft);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	// create graph
	// use cout?

	auto block {[] (const std::string & n, std::list <std::string> & p, const std::string & add)
	{
		if (p.size() == 1)
		{
			std::cout << n << " -> " << *p.begin();

			if (add.length() > 0)
				std::cout << '[' << add << ']';

			std::cout << ";\n";
		}
		else if (p.size() != 0)
		{
			std::cout << n << " -> { ";
			for (auto & link : p)
			{
				std::cout << link << " ";
				//std::cout << i.first << " -> " << link << ";\n";
			}

			if (add.length() > 0)
				std::cout << "} [" << add << "];\n";
			else
				std::cout << "};\n";
		}
	}};

	std::cout << "digraph chapter_view {\n";
	for (auto & i : gData)
	{
		block (i.first, i.second.links, "");
		block (i.first, i.second.breakTos, "color=blue");
	}
	std::cout << "}" << std::endl;

	return 0;
}
