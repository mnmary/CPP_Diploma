#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <regex>
#include <map>
#include <algorithm>

/*
Утилиты
*/

//список символов для проверки слова
const std::string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

enum class TProtocol
{
	//типы протоколов
	HTTP = 0,
	HTTPS = 1
};

struct Link
{
	//ссылка
	TProtocol protocol;
	std::string host;
	std::string query;

	bool operator==(const Link& l) const
	{
		return protocol == l.protocol
			&& host == l.host
			&& query == l.query;
	}
};

struct TqueueItem
{
	//элемент очереди ссылок
	std::string url;
	int dept;

	bool operator==(const TqueueItem& l) const
	{
		return url == l.url
			&& dept == l.dept;
	}
};

bool isAlpha(char s)
{
	if (letters.find_first_of(s) != std::string::npos)
	{
		return true;
	}
	return false;
}

//trim
void trim(std::string& s)
{
	//убираем пробелы в начале и в конце строки
	s.erase(0, s.find_first_not_of(" \t"));
	s.erase(s.find_last_not_of(" \t") + 1);
}

void toLower(std::string& value)
{
	//преобразуем строки в символы нижнего регистра
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) 
		{ 
			return std::tolower(c); 
		}
	);
}

std::string urlDecode(const std::string& encoded) 
{
	std::string res;
	std::istringstream iss(encoded);
	char ch;

	while (iss.get(ch)) 
	{
		if (ch == '%') 
		{
			int hex;
			iss >> std::hex >> hex;
			res += static_cast<char>(hex);
		}
		else 
		{
			res += ch;
		}
	}

	return res;
}

std::string convert_to_utf8(const std::string& str) 
{
	std::string url_decoded = urlDecode(str);
	return url_decoded;
}

std::string LinkToURL(Link link)
{
	//преобразуем структуру ссылки в строку
	if (link.protocol == TProtocol::HTTP)
	{
		return("http://" + link.host + link.query);
	}
	else
	{
		return("https://" + link.host + link.query);
	}

}