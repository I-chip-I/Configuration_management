#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <Lmcons.h>
using namespace std;

void Get_name(char* user_name, char* host_name)  //получение реальных данных, используемых для формирования приглашения к вводу
{
    DWORD u_size = UNLEN + 1;
    DWORD h_size = MAX_COMPUTERNAME_LENGTH + 1;

    GetUserNameA(user_name, &u_size);
    GetComputerNameA(host_name, &h_size);
}

string Invite(char* user_name, char* host_name) //приглашение к вводу и считывание введённой команды
{
    cout << user_name << "@" << host_name << ":~$ ";
    string command;
    getline(cin, command);
    return command;
}

void Parser(string arg, vector<string>& func_arguments) //обработка аргументов введённой команды
{
    for (int i = 0; i < arg.size(); i++)
    {
        string result_arg = "";

        if (arg[i] == '"') //проверка двойных кавычек
        {
            int j = i + 1;
            int i_quotation_mark2 = arg.find('"', j);

            if (i_quotation_mark2 >= 0) //найдена вторая кавычка
            {
                while (arg[j] != '"')
                {
                    result_arg += arg[j];
                    j++;
                }
                func_arguments.push_back(result_arg);
                i = j;
            }
            else //не найдена вторая кавычка
            {
                int j = i;

                while (arg[j] != ' ')
                {
                    result_arg += arg[j];
                    j++;
                    if (j == arg.size())
                    {
                        break;
                    }
                }
                func_arguments.push_back(result_arg);
                i = j;
            }
        }
        else if (arg[i] == '\'') //проверка одинарных кавычек
        {
            int j = i + 1;
            int i_quotation_mark2 = arg.find('\'', j);

            if (i_quotation_mark2 >= 0) //найдена вторая кавычка
            {
                while (arg[j] != '\'')
                {
                    result_arg += arg[j];
                    j++;
                }
                func_arguments.push_back(result_arg);
                i = j;
            }
            else //не найдена вторая кавычка
            {
                int j = i;

                while (arg[j] != ' ')
                {
                    result_arg += arg[j];
                    j++;
                    if (j == arg.size())
                    {
                        break;
                    }
                }
                func_arguments.push_back(result_arg);
                i = j;
            }
        }
        else if (arg[i] == ' ') //проверка пробела
        {
            continue;
        }
        else //проверка других случаев
        {
            int j = i;

            while (arg[j] != ' ')
            {
                result_arg += arg[j];
                j++;
                if (j == arg.size())
                {
                    break;
                }
            }
            func_arguments.push_back(result_arg);
            i = j;
        }
    }
}

void Error_message(int error_code, string command) //сообщение о некорректно введённой команде
{
    switch (error_code)
    {
    case 1:
        cout << command << ": command not found";
        break;
    }
}

void ls(string command) //команда ls
{
    if (command.length() > 2)
    {
        if (command[2] == ' ')
        {
            vector<string> func_arguments;
            Parser(command.substr(3), func_arguments);
            cout << "ls ";
            for (int i = 0; i < func_arguments.size(); i++)
            {
                cout << func_arguments[i] << " ";
            }
        }
        else
        {
            Error_message(1, command);
        }
    }
    else
    {
        cout << "ls";
    }
}

void cd(string command) //команда cd
{
    if (command.length() > 2)
    {
        if (command[2] == ' ')
        {
            vector<string> func_arguments;
            Parser(command.substr(3), func_arguments);
            cout << "cd ";
            for (int i = 0; i < func_arguments.size(); i++)
            {
                cout << func_arguments[i] << " ";
            }
        }
        else
        {
            Error_message(1, command);
        }
    }
    else
    {
        cout << "cd";
    }
}

int main()
{
    setlocale(LC_ALL, "rus");

    char user_name[UNLEN + 1];
    char host_name[MAX_COMPUTERNAME_LENGTH + 1];

    Get_name(user_name, host_name);
    string command;
    while (true)
    {
        cout << endl;
        command = Invite(user_name, host_name);
        if (command == "exit")
        {
            break;
        }
        else if (command[0] == 'l' and command[1] == 's')
        {
            ls(command);
            continue;
        }
        else if (command[0] == 'c' and command[1] == 'd')
        {
            cd(command);
            continue;
        }
        else if (command.size() > 0)
        {
            Error_message(1, command.substr(0, command.find(' ')));
            continue;
        }
    }
}