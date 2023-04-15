#include "../inc/servme.hpp"

std::string	normalizePath(std::string	path)
{
	std::stack<std::string>	paths;

	std::string	toPush;
	size_t	index;
	size_t	index2;
	std::string	toPop;
	std::string	newStr;

	while (!path.empty())
	{
		index = path.find_first_of('/');
		if (index == 0)
		{
			toPush = path.substr(index + 1);
			path = path.substr(index + 1);
			index2 = path.find_first_of('/');
			if (index2 != std::string::npos)
			{
				toPush = toPush.substr(0, index2);
				path = path.substr(index2);
			}
			else
				path = "";
			if (toPush == ".")
				continue ;
			if (toPush == "..")
			{
				if (!paths.empty())
					paths.pop();
			}
			else
				paths.push(toPush);
		}
	}
	if (paths.empty())
		paths.push("/");	
	while (!paths.empty())
	{
		toPop = paths.top();
		if (toPop == "/")
			newStr = toPop + newStr;
		else
			newStr = "/" + toPop + newStr;
		paths.pop();
	}
	return (newStr);
}