#pragma once
#include <cctype>
#include <string>
#include <string_view>


inline std::string_view trim_whitespace (std::string_view in)
{
	auto start = in.cbegin();
	auto end = in.cend();

	while (start != end) {
		if (!std::isspace(*start)) {
			break;
		}
		start++;
	}

	while (end != start) {
		if (!std::isspace(*end)) {
			break;
		}
		end--;
	}

	const auto len = end - start;
	return std::string_view(start, len);
}

inline void to_lower_inline (std::string & in)
{
	std::transform (in.begin(), in.end(), in.begin(), ::tolower);
}

inline std::string to_lower (std::string in)
{
	std::transform (in.begin(), in.end(), in.begin(), ::tolower);
	return in;
}

inline std::string make_safe (std::string_view in)
{
	std::string a (trim_whitespace (in));
	to_lower_inline (a);

	std::transform (a.begin(), a.end(), a.begin(), [] (char c)
	{
		if (not std::isalnum (c))
			return '_';
		return c;
	});

	return a;
}

bool cmp_lower(std::string_view lhs, std::string_view rhs)
{
	if (lhs.length() != rhs.length())
	{
		return false;
	}

	for(size_t i = 0; i < lhs.length(); i++)
	{
		if (std::tolower(lhs[i]) != rhs[i])
		{
			return false;
		}
	}
	return true;
}
