#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "../Include/root_certificates.hpp"
#include "../Include/DB_Client.h"
#include "../Include/HTTP_HTTPs_Client.h"
#include "../Include/INI_file.h"
#include "../Include/Utils.h"
#include "../Include/Safe_Queue.h"
#include "../Include/HTML_parser.h"

/*
# ��������� �����
��������� ����� (spider), ��� �������� (crowler), � ��� ������������ ����� ��������� �������. 
��� ���������, ������� ������� ��������, ������ �� ������� �� ���-���������. ������� � ����� ��������, 
��� ��������� �� ������ �� �������, ��������� �� ���� ���������. 
����� ��������� ���������� ������ ��������, � ����� ����������� �.
*/

//��� �������
std::vector<std::thread> arrThread;
//������� ������� �� ������ �� ����������
Safe_Queue <TqueueItem> queue;
//�������
std::mutex mutex;
//���� ���������� ������
bool stop{ false };


//��� ������� ��������� ���������� �������� �� ������
void doWork(DB_Client& db)
{
	std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
	while (!stop)
	{
		TqueueItem work;
		std::unique_lock<std::mutex> lock(mutex);
		
		if (!queue.empty())
		{
			//��������� ������ �� ����������
			work = queue.pop();
			std::cout << std::this_thread::get_id() << " POP " << work.url << " depth = " << work.dept << " " << "\n";
			
			HTML_Parser parser(db, work.url);
			if (parser.parse())
			{
				std::cout << "link = " << parser.GetURL() << " links count " << parser.getSubLinks().size() << std::endl;
				//���� ������� �������� �������� ��� �� 0, �� ��� ������ �� �������� ���� �������� � ������� �� ���������� � �������� �������� -1
				if (work.dept > 0)
				{
					//std::cout << "depth > 0 links count = " << parser.getSubLinks().size() << std::endl;
					for (auto& subLink : parser.getSubLinks())
					{
						if (!db.findUrl(subLink))
						{
							//���� ��� ����� ������ � ���� ������
							TqueueItem qSubLink;
							qSubLink.url = subLink;
							qSubLink.dept = work.dept - 1;
							//����� ������ �� ����������� �������� �������� -1
							queue.push(qSubLink);
							//std::cout << std::this_thread::get_id() << " PUSH " << qSubLink.url << " depth = " << qSubLink.dept << "\n";
						}
					}
				}
			}
			
		}
		
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

int main()
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	try
	{
		/*
		## ���� ������������ ini
		��� ��������� �������� ������� ������� � ini-�����. ���� ���� ������ ��������� ��� ������� ����� ���������.

		� ���� ����� ������� ������� ��������� ���������:
		* ���� (�����) ���� ������;
		* ����, �� ������� �������� ���� ������;
		* �������� ���� ������;
		* ��� ������������ ��� ����������� � ���� ������;
		* ������ ������������ ��� ����������� � ���� ������;
		* ��������� �������� ��� ��������� �����;
		* ������� �������� ��� ��������� �����;
		* ���� ��� ������� ���������-����������.		
		*/
		INI_file ini_file("../config.ini");

		std::string start_page = ini_file.get_value("Client.start_page");
		int recursion_depth = std::stoi(ini_file.get_value("Client.recursion_depth"));

		std::string host = ini_file.get_value("DataBase.bd_host");
		std::string port = ini_file.get_value("DataBase.bd_port");
		std::string name = ini_file.get_value("DataBase.bd_name");
		std::string user = ini_file.get_value("DataBase.bd_user");
		std::string password = ini_file.get_value("DataBase.bd_pass");

		/*
		## ���� ������
		��� ���������� ������� ������������� ������������ ���� ������ PostgreSQL.
		��������� ���� ������ (����, ����, �������� ��, ��� ������������, ������) ������� ������� � ini-����� ������������.
		��� �������� ���������� � ����������� ���� ������������� ������������ ��� ������� � ����������� � ������ � 
		� ����������� ����� ���� ����� �������-��-������ � ������� ������������� �������. 
		� ��� �� ������������� ������� ����� �������� ������� ������������� ����� � ���������.		
		*/
		DB_Client db(host, port, name, user, password);

		/*
		## ����������
		� HTML-��������� ����� ���������� ������, ������������ HTML-����� `<a href="...">`. 
		����� ������ ���������� �� ���� ������� � ��������� ��� �������� ����. 
		��� ����� ����� ��������� ������������� ������� � ��� �������. 
		*/
		int cntPool = std::thread::hardware_concurrency();
		for (int i = 0; i < cntPool; i++)
		{
			arrThread.push_back(std::thread(&doWork, db));
		}

		/*
		� ini-����� ������������ ������� ��������, ������� ���������� ���������, � ����� �������� ���� ������ ������ � ��.
		������� ������� �� ������ �� ����������, � ��� ������� ��������� ������
		�� ���������� ���-������� � ���� ������ � �� ����������.
		����� ���������� ��������� �������� ����� �������� ��� ������ � �� � ��������� �� � ������� �� ����������.
		*/		
		TqueueItem qFirstLink;
		qFirstLink.url = start_page;
		qFirstLink.dept = recursion_depth;
		
		/*
		����� ��������� ��� �������� � ����������� �.
		*/
		queue.push(qFirstLink);
		std::cout << std::this_thread::get_id() << " PUSH " << qFirstLink.url << " depth = " << qFirstLink.dept << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(2));
		
		//destroy		
		std::unique_lock<std::mutex> lock(mutex);
		stop = true;
		lock.unlock();

		for (auto& t : arrThread)
		{
			t.join();
		}


	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	system("pause");
	return 0;
}