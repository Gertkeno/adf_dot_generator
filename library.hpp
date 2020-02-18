#pragma once

inline std::string trim_whitespace (const std::string & in)
{
	unsigned index {0};
	while (index < in.length() and std::isspace (in [index]))
		index++;

	if (index >= in.length() or index == 0)
		return in;

	return in.substr (index);
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
