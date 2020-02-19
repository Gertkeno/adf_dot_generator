#pragma once

inline std::string trim_front_whitespace (const std::string & in)
{
	unsigned index {0};
	while (index < in.length() and std::isspace (in [index]))
		++index;

	if (index >= in.length() or index == 0)
		return in;

	return in.substr (index);
}

inline std::string trim_back_whitespace (const std::string & in)
{
	long unsigned index {in.length()-1};
	while (index != 0 and std::isspace (in [index]))
		--index;

	if (index >= in.length()-1 or index == 0)
		return in;

	return in.substr (0, index+1);
}

inline std::string trim_whitespace (const std::string & in)
{
	return trim_back_whitespace (trim_front_whitespace (in));
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

inline std::string make_safe (std::string in)
{
	in = trim_whitespace (in);
	to_lower_inline (in);

	std::transform (in.begin(), in.end(), in.begin(), [] (char c)
	{
		if (std::isspace (c))
			return '_';
		return c;
	});

	return in;
}
