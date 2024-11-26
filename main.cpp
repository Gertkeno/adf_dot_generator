#include <fstream>
#include <string>
#include <string_view>
#include <iostream>
#include <list>
#include <unordered_map>
#include <algorithm>

#include "library.hpp"

// ==12861==   total heap usage: 1,931 allocs, 1,931 frees, 162,647 bytes allocated
// ==14239==   total heap usage:   283 allocs,   283 frees, 103,695 bytes allocated

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

			std::string_view view = trim_whitespace (line);
			switch (view [0])
			{
			case '#':
				{
					const auto preceedingMarks (view.find_first_not_of ('#'));
					const auto name {make_safe (view.substr (preceedingMarks))};
					writingTo = &gData [name];
					writingTo->name = name;
					break;
				}
			case '~':
				if (writingTo != nullptr)
				{
					const auto p {view.find (':')};
					if (p == std::string::npos or p+2 >= view.length())
						break;

					const auto fn {view.substr (1, p-1)};
					const auto ft {make_safe (view.substr (p+1))};

					if (cmp_lower(fn, "link") or cmp_lower(fn, "tag") or cmp_lower(fn, "linkto"))
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

	auto block {[] (std::string_view n, const std::list <std::string> & p, std::string_view add)
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
		if (i.second.links.size() == 0 and i.second.breakTos.size() == 0)
		{
			std::cout << i.first << ";\n";
			continue;
		}

		block (i.first, i.second.links, "");
		block (i.first, i.second.breakTos, "color=blue");
	}
	std::cout << "}" << std::endl;

	return 0;
}
