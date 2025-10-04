#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <Lmcons.h>
#include <fstream>
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

string Command_to_string(int q_commands, char* command[]) //перевод формата команды из массива символов в строку
{
    string string_command = "";
    for (int i = 1; i < q_commands; i++)
    {
        string string_path = command[i];
        if (string_path.length() > 2 and command[i][0] == 'C' and command[i][1] == ':')
        {
            string_path = "\"" + string_path + "\"";
        }

        if (i != q_commands - 1)
        {
            string_command = string_command + string_path + " ";
        }
        else
        {
            string_command = string_command + string_path;
        }
    }

    return string_command;
}

string Skip_spaces(string command) //пропуск лишних пробелов в начале строки при её считывании
{
    while (command[0] == ' ')
    {
        command = command.substr(1);
    }

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

void Error_message(int error_code, string command) //вывод сообщения об ошибке
{
    switch (error_code)
    {
    case 1:
        cout << command << ": command not found";
        break;
    case 2:
        if (command.length() > 0)
        {
            cout << endl << "\"" << command << "\"" << ": Permission denied or path not correct" << endl;
        }
        else
        {
            cout << endl << "Permission denied or path not correct" << endl;
        }
        break;
    case 3:
        if (command.length() > 0)
        {
            cout << command << ": No such file or directory";
        }
        else
        {
            cout << "No such file or directory";
        }
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

void VFS(string parameter) //команда --vfs
{
    if (parameter.length() > 0)
    {
        cout << endl;
        cout << "VFS: " << parameter << endl;
    }
    else
    {
        Error_message(3, parameter);
    }
}

void Script(string f_path, char* user_name, char* host_name) //команда --script
{
    ifstream in(f_path);
    if (in.is_open())
    {
        string command = "";

        while (getline(in, command))
        {
            cout << endl;
            cout << user_name << "@" << host_name << ":~$ " << command << endl;
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

        in.close();
    }
    else
    {
        Error_message(2, f_path);
    }
}

void Configuration_file(string f_path, char func, char* user_name, char* host_name) //команда --config
{
    ifstream in(f_path);
    if (in.is_open())
    {
        string command = "";

        while (getline(in, command))
        {
            command = Skip_spaces(command);

            if ((func == 'a' or func == 'v') and command.find("<VFS>") == 0)
            {
                command = command.substr(5, command.length() - 5 - 6);
                cout << "--vfs" << " " << command << endl;
                VFS(command);
                continue;
            }
            else if ((func == 'a' or func == 's') and command.find("<Script>") == 0)
            {
                command = command.substr(8, command.length() - 8 - 9);
                cout << "--script" << " " << command << endl;
                Script(command, user_name, host_name);
                continue;
            }
            else
            {
                continue;
            }
        }

        in.close();
    }
    else
    {
        Error_message(2, f_path);
    }
}

void Launched_with_command(int q_commands, char* with_command[], char* user_name, char* host_name) //запуск режима ввода параметров в приложение
{
    string command = Command_to_string(q_commands, with_command);

    vector<string> func_arguments;
    Parser(command, func_arguments);

    if (func_arguments[0] == "--vfs" and func_arguments.size() == 2)
    {
        cout << func_arguments[0] << " " << func_arguments[1] << endl;
        VFS(func_arguments[1]);
    }
    else if (func_arguments[0] == "--script" and func_arguments.size() == 2)
    {
        cout << func_arguments[0] << " " << func_arguments[1] << endl;
        Script(func_arguments[1], user_name, host_name);
    }
    else if (func_arguments[0] == "--config")
    {
        cout << func_arguments[0];

        if (func_arguments.size() == 1)
        {
            cout << endl;
            func_arguments.push_back("C:\\Users\\Света\\Desktop\\Configuration_management\\Practical_work_1\\Scripts and files\\Configuration.xml");
        }
        else
        {
            cout << " " << func_arguments[1] << endl;
        }
        char func = 'a';

        Configuration_file(func_arguments[1], func, user_name, host_name);
    }
    else if ((func_arguments[0] == "--vfs" or func_arguments[0] == "--script") and func_arguments.size() == 1)
    {
        cout << func_arguments[0] << endl;

        func_arguments.push_back("C:\\Users\\Света\\Desktop\\Configuration_management\\Practical_work_1\\Scripts and files\\Configuration.xml");
        char func;

        if (func_arguments[0] == "--vfs")
        {
            func = 'v';
        }
        else
        {
            func = 's';
        }

        cout << "--conf" << " " << func_arguments[1] << endl;
        Configuration_file(func_arguments[1], func, user_name, host_name);
    }
    else if (func_arguments[0] == "ls")
    {
        ls(command);
    }
    else if (func_arguments[0] == "cd")
    {
        cd(command);
    }
    else
    {
        Error_message(1, command.substr(0, command.find(' ')));
    }
}

void Launched_without_command(char* user_name, char* host_name) //запуск режима непосредственного диалога с пользователем
{
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

int main(int q_commands, char* with_command[])
{
    setlocale(LC_ALL, "rus");

    char user_name[UNLEN + 1];
    char host_name[MAX_COMPUTERNAME_LENGTH + 1];
    Get_name(user_name, host_name);

    if (q_commands > 1)
    {
        Launched_with_command(q_commands, with_command, user_name, host_name);
    }
    else
    {
        Launched_without_command(user_name, host_name);
    }
}