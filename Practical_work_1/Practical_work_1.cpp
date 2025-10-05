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
    case 4:
        cout << command << ": not a zip archive" << endl;
        break;
    case 5:
        cout << command << ": empty" << endl;
        break;
    }
}

struct Entry
{
public:
    string name;
    string path;
    bool is_folder = 1;
    Entry* Up_folder;
    vector<Entry*> Down_entries;

    void Rename_entry(vector<Entry>& Zip_entries, bool is_in_vector)
    {
        int i_slash = path.rfind('/');
        if (i_slash != -1)
        {
            name = path.substr(i_slash + 1);
            string up_folder_name = "";

            int i_slash_2 = (path.substr(0, i_slash)).rfind('/');
            if (i_slash_2 != -1)
            {
                up_folder_name = path.substr(i_slash_2 + 1, (i_slash - 1) - i_slash_2);
            }
            else
            {
                up_folder_name = path.substr(0, i_slash);
            }

            if (is_in_vector == 0)
            {
                Zip_entries.push_back(*this);
                Find_up_folder(i_slash, up_folder_name, Zip_entries);
            }
        }
        else
        {
            name = path;
            Up_folder = nullptr;

            if (is_in_vector == 0)
            {
                Zip_entries.push_back(*this);
            }
        }
    }

    void Find_up_folder(int i_slash, string up_folder_name, vector<Entry>& Zip_entries)
    {
        for (int i = 0; i < Zip_entries.size(); i++)
        {
            if (Zip_entries[i].name == up_folder_name)
            {
                Up_folder = &Zip_entries[i];
                Up_folder->Down_entries.push_back(this);
                return;
            }

            if (Zip_entries[i].name == name)
            {
                Entry new_entry;
                new_entry.name = up_folder_name;
                new_entry.path = path.substr(0, i_slash);
                Up_folder = &new_entry;
                Up_folder->Down_entries.push_back(this);
                Zip_entries.insert(Zip_entries.begin() + i, new_entry);
                new_entry.Rename_entry(Zip_entries, 1);
                return;
            }
        }
    }
};

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

size_t Read_zip(string f_path, vector<unsigned char>& Zip_data) //чтение zip-файла
{
    ifstream in(f_path, ios::binary);
    if (in.is_open())
    {
        in.seekg(0, ios::end);
        size_t f_size = in.tellg();
        in.seekg(0, ios::beg);

        Zip_data.resize(f_size);
        in.read((char*)Zip_data.data(), f_size);

        in.close();
        return f_size;
    }
    else
    {
        Error_message(2, f_path);
        return -1;
    }
}

void VFS(string f_path) //команда --vfs
{
    int is_zip = f_path.find(".zip");
    if (is_zip != -1)
    {
        if (f_path.length() > 0)
        {
            vector<unsigned char> Zip_data;
            size_t f_size = Read_zip(f_path, Zip_data);

            if (f_size > 22)
            {
                int pos_centre_dir = -1, i = f_size - 22; //сигнатура начинается не позже 23 байта от конца файла
                while (i >= 0)
                {
                    if (Zip_data[i + 3] == 0x06 && Zip_data[i + 2] == 0x05 && Zip_data[i + 1] == 0x4b && Zip_data[i] == 0x50) //0x06054b50 последовательность байтов, означающая конец центрального каталога
                    {
                        pos_centre_dir = i;
                        break;
                    }
                    i--;
                }


                if (pos_centre_dir != -1)
                {
                    vector<Entry> Zip_entries;
                    i = 0;

                    while (i < pos_centre_dir)
                    {
                        if (Zip_data[i] == 0x50 && Zip_data[i + 1] == 0x4b && Zip_data[i + 2] == 0x01 && Zip_data[i + 3] == 0x02)
                        {
                            int len_name = Zip_data[i + 28] | (Zip_data[i + 29] << 8); //сдвиг старшего байта вперёд, так как читается число аналогично сигнатуре справа налево

                            Entry new_entry;
                            new_entry.path = "";

                            for (int j = 0; j < len_name; j++)
                            {
                                new_entry.path = new_entry.path + (char)Zip_data[(i + 46) + j];
                            }


                            if (new_entry.path.back() != '/')
                            {
                                new_entry.is_folder = 0;
                            }
                            else
                            {
                                new_entry.path = new_entry.path.substr(0, new_entry.path.length() - 1);
                            }

                            new_entry.Rename_entry(Zip_entries, 0);
                        }
                        i++;
                    }

                    cout << "VFS:" << endl;
                    for (int i = 0; i < Zip_entries.size(); i++)
                    {
                        if (Zip_entries[i].is_folder == 1)
                        {
                            cout << "Folder: " << Zip_entries[i].name << endl;
                            cout << "Folder: " << Zip_entries[i].path << endl;
                        }
                        else
                        {
                            cout << "Folder: " << Zip_entries[i].name << endl;
                            cout << "File: " << Zip_entries[i].path << endl;
                        }
                    }
                }
                else if (f_size == -1)
                {
                    return;
                }
                else
                {
                    Error_message(5, f_path);
                }
            }
            else
            {
                Error_message(5, f_path);
            }

        }
        else
        {
            Error_message(3, f_path);
        }
    }
    else
    {
        Error_message(4, f_path);
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
                VFS(command);
                continue;
            }
            else if ((func == 'a' or func == 's') and command.find("<Script>") == 0)
            {
                command = command.substr(8, command.length() - 8 - 9);
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
        VFS(func_arguments[1]);
    }
    else if (func_arguments[0] == "--script" and func_arguments.size() == 2)
    {
        Script(func_arguments[1], user_name, host_name);
    }
    else if (func_arguments[0] == "--config")
    {
        if (func_arguments.size() == 1)
        {
            func_arguments.push_back("C:\\Users\\Света\\Desktop\\Configuration_management\\Practical_work_1\\Scripts and files\\Configuration.xml");
        }
        char func = 'a';

        Configuration_file(func_arguments[1], func, user_name, host_name);
    }
    else if ((func_arguments[0] == "--vfs" or func_arguments[0] == "--script") and func_arguments.size() == 1)
    {
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