#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <Lmcons.h>
#include <fstream>
#include <ctime>
#include <thread>
#include <iomanip>
#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")
using namespace std;

struct Entry;
bool VFS_loaded = 0;

//глобальные переменные для хранения указателей
Entry* last_entry = nullptr;
Entry* current_entry = nullptr;
Entry* temp = nullptr;
vector<Entry*> Zip_entries;

void Error_message(int error_code, string command);

struct Entry //структура папок/файлов VFS
{
public:
    string name;
    string path;
    bool is_folder = 1;
    Entry* Up_folder;
    vector<Entry*> Down_entries;

    void Rename_entry(bool is_in_vector)
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
                if (i_slash == 0)
                {
                    up_folder_name = "/";
                    Find_up_folder(i_slash, up_folder_name);
                }
                else
                {
                    up_folder_name = path.substr(0, i_slash);
                }
            }

            path = "/" + path;

            if (is_in_vector == 0)
            {
                Zip_entries.push_back(this);
                Find_up_folder(i_slash, up_folder_name);
            }

            bool is_found = 0;
            int index = -1;
            for (int i = 0; i < Zip_entries.size(); i++)
            {
                if (Zip_entries[i]->name == up_folder_name)
                {
                    index = i;
                    for (int j = 0; j < Zip_entries[i]->Down_entries.size(); j++)
                    {
                        if (Zip_entries[i]->Down_entries[j]->name == name)
                        {
                            is_found = 1;
                            break;
                        }
                    }
                }
            }
            if (is_found == 0)
            {
                Zip_entries[index]->Down_entries.push_back(this);
            }
        }
        else
        {
            name = path;
            path = "/" + path;
            Up_folder = Zip_entries[0];
            Up_folder->Down_entries.push_back(this);

            if (is_in_vector == 0)
            {
                Zip_entries.push_back(this);
            }
        }
    }

    void Find_up_folder(int i_slash, string up_folder_name)
    {
        for (int i = 0; i < Zip_entries.size(); i++)
        {
            if (Zip_entries[i]->name == up_folder_name)
            {
                Up_folder = Zip_entries[i];
                Up_folder->Down_entries.push_back(this);
                return;
            }

            if (Zip_entries[i]->name == name)
            {
                Entry* new_entry = new Entry;
                new_entry->name = up_folder_name;
                new_entry->path = path.substr(0, i_slash + 1);
                Up_folder = new_entry;
                Up_folder->Down_entries.push_back(this);
                Zip_entries.insert(Zip_entries.begin() + i, new_entry);
                new_entry->Rename_entry(1);
                return;
            }
        }
    }


    void Current_entry_by_relative_path(string path, string orig_path)
    {
        int i_slash = path.find('/');

        if (i_slash != -1)
        {
            bool is_found = 0;

            for (int i = 0; i < temp->Down_entries.size(); i++)
            {
                if (temp->Down_entries[i]->name == path.substr(0, i_slash))
                {
                    temp = temp->Down_entries[i];
                    is_found = 1;
                    Current_entry_by_relative_path(path.substr(i_slash + 1), orig_path);
                }
            }

            if (is_found == 0)
            {
                Error_message(3, orig_path);
            }
        }
        else
        {
            for (int i = 0; i < temp->Down_entries.size(); i++)
            {
                if (temp->Down_entries[i]->name == path)
                {
                    current_entry = temp->Down_entries[i];
                    return;
                }
            }

            Error_message(3, orig_path);
            return;
        }

    }

    void Print_by_relative_path(string path, string orig_path)
    {
        int i_slash = path.find('/');

        if (i_slash != -1)
        {
            bool is_found = 0;

            for (int i = 0; i < temp->Down_entries.size(); i++)
            {
                if (temp->Down_entries[i]->name == path.substr(0, i_slash))
                {
                    temp = temp->Down_entries[i];
                    is_found = 1;
                    Print_by_relative_path(path.substr(i_slash + 1), orig_path);
                }
            }

            if (is_found == 0)
            {
                Error_message(3, orig_path);
            }
        }
        else
        {
            if (temp->Down_entries.size() != 0)
            {
                for (int i = 0; i < temp->Down_entries.size(); i++)
                {
                    cout << temp->Down_entries[i]->name << " ";
                }
                return;

            }
            else
            {
                if (temp->is_folder == 0)
                {
                    Error_message(5, temp->name);
                    return;
                }
                else
                {
                    Error_message(5, temp->name);
                    return;
                }
            }
        }

    }
};

void Get_name(char* user_name, char* host_name)  //получение реальных данных, используемых для формирования приглашения к вводу
{
    DWORD u_size = UNLEN + 1;
    DWORD h_size = MAX_COMPUTERNAME_LENGTH + 1;

    GetUserNameA(user_name, &u_size);
    GetComputerNameA(host_name, &h_size);
}

int Count_users() //подсчёт пользователей
{
    WTS_SESSION_INFO* sessions = nullptr;
    DWORD q_sessions = 0;
    int q_users = 0;

    WTS_CONNECTSTATE_CLASS* state = nullptr;
    DWORD skip_part;

    if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions, &q_sessions))
    {
        for (DWORD i = 0; i < q_sessions; i++)
        {
            if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sessions[i].SessionId, WTSConnectState, (LPWSTR*)&state, &skip_part))
            {
                if (*state == WTSActive)
                {
                    q_users++;
                }

                WTSFreeMemory(state);
            }

        }

        WTSFreeMemory(sessions);
    }

    return q_users;
}

double CPU_load() //эмуляция получения данных о нагрузке процессора, так как на Windows нет возможности обратиться с запросом реальных данных о нагрузке процессора за последнюю минуту, 5 или 15 минут
{
    //создание структуры FILETIME для хранения времени в двух виде 32-битных чисел, являющихся младшими и старшими 32 битами реального числа
    FILETIME idle_time_beg, kernel_time_beg, user_time_beg; //время отсутствия нагрузки, время нагрузки в режиме ядра, время нагрузки в режиме работы с пользователем
    FILETIME idle_time_end, kernel_time_end, user_time_end;

    GetSystemTimes(&idle_time_beg, &kernel_time_beg, &user_time_beg);

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    GetSystemTimes(&idle_time_end, &kernel_time_end, &user_time_end);

    //приведение полученных данных к безнаковому 64-битному целому числу
    ULONGLONG new_idle_time_beg = ((ULONGLONG)idle_time_beg.dwHighDateTime << 32) | idle_time_beg.dwLowDateTime;
    ULONGLONG new_idle_time_end = ((ULONGLONG)idle_time_end.dwHighDateTime << 32) | idle_time_end.dwLowDateTime;
    ULONGLONG new_kernel_time_beg = ((ULONGLONG)kernel_time_beg.dwHighDateTime << 32) | kernel_time_beg.dwLowDateTime;
    ULONGLONG new_kernel_time_end = ((ULONGLONG)kernel_time_end.dwHighDateTime << 32) | kernel_time_end.dwLowDateTime;
    ULONGLONG new_user_time_beg = ((ULONGLONG)user_time_beg.dwHighDateTime << 32) | user_time_beg.dwLowDateTime;
    ULONGLONG new_user_time_end = ((ULONGLONG)user_time_end.dwHighDateTime << 32) | user_time_end.dwLowDateTime;
    ULONGLONG idle_difference = new_idle_time_end - new_idle_time_beg;
    ULONGLONG total_difference = (new_kernel_time_end - new_kernel_time_beg) + (new_user_time_end - new_user_time_beg);

    if (total_difference != 0)
    {
        double part_of_idle_time_to_total_time = (double)idle_difference / total_difference; //доля времени отсутствия нагрузки (это время является частью времени нагрузки в режиме ядра)
        double cpu_load_per_5_sec = 1.0 - part_of_idle_time_to_total_time; //доля времени нагрузки процессора
        return cpu_load_per_5_sec;
    }
}

void Invite(char* user_name, char* host_name) //приглашение к вводу
{
    if (current_entry != nullptr && current_entry->path != "/")
    {
        cout << user_name << "@" << host_name << ":" << current_entry->path << "$ ";
    }
    else
    {
        cout << user_name << "@" << host_name << ":~$ ";
    }
}

string Input() //считывание команды с параметром в режиме непосредственного диалога с пользователем
{
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

bool Is_command(string command) //проверка на то, является ли переданный параметр командой
{
    vector<string> commands = { "--vfs", "--script", "--config", "ls", "cd", "clear", "uptime" };

    for (int i = 0; i < commands.size(); i++)
    {
        if (commands[i] == command)
        {
            return true;
        }
    }

    return false;
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
        cout << command.substr(0, command.find(' ')) << ": command not found" << endl;
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
            cout << command << ": No such file or directory" << endl;
        }
        else
        {
            cout << "No such file or directory" << endl;
        }
        break;
    case 4:
        cout << command << ": not a zip archive" << endl;
        break;
    case 5:
        cout << command << ": empty" << endl;
        break;
    case 6:
        cout << command << ": not a directory" << endl;
        break;
    case 7:
        cout << command.substr(0, 5) << ": extra operand '" << command.substr(6) << "'" << endl;
        break;
    case 8:
        cout << "File system not available" << endl;
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

            if (func_arguments[0] == "/" && func_arguments.size() == 1)
            {
                if (Zip_entries[0]->Down_entries.size() != 0)
                {
                    for (int i = 0; i < current_entry->Down_entries.size(); i++)
                    {
                        cout << current_entry->Down_entries[i]->name << " ";
                    }

                    cout << endl;
                    return;
                }
                else
                {
                    Error_message(5, func_arguments[0]);
                }
            }
            else
            {
                temp = current_entry;
                temp->Print_by_relative_path(func_arguments[0], func_arguments[0]);
                cout << endl;
            }
        }
        else
        {
            Error_message(1, command);
            return;
        }
    }
    else
    {
        if (current_entry != nullptr)
        {
            if (current_entry->Down_entries.size() != 0)
            {
                for (int i = 0; i < current_entry->Down_entries.size(); i++)
                {
                    cout << current_entry->Down_entries[i]->name << " ";
                }
                cout << endl;

                return;
            }
            else
            {
                if (current_entry->is_folder == 0)
                {
                    Error_message(6, current_entry->name);
                    return;
                }
                else
                {
                    Error_message(5, current_entry->name);
                    return;
                }
            }
        }
        else
        {
            Error_message(3, "");
            return;
        }
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

            if ((func_arguments[0] == "~" || func_arguments[0] == "/") && func_arguments.size() == 1)
            {
                last_entry = current_entry;
                current_entry = Zip_entries[0];
                return;
            }
            else if (func_arguments[0] == ".." && func_arguments.size() == 1)
            {
                if (current_entry->Up_folder != nullptr)
                {
                    last_entry = current_entry;
                    current_entry = current_entry->Up_folder;
                    return;
                }
                Error_message(3, func_arguments[0]);
            }
            else if (func_arguments[0] == "." && func_arguments.size() == 1)
            {
                current_entry = current_entry;
                return;
            }
            else if (func_arguments[0] == "-" && func_arguments.size() == 1)
            {
                current_entry = last_entry;
                return;
            }
            else if (func_arguments[0].length() > 0 && func_arguments.size() == 1)
            {
                if (func_arguments[0][0] == '/' && func_arguments[0][1] != '/')
                {
                    for (int i = 0; i < Zip_entries.size(); i++)
                    {
                        if (Zip_entries[i]->path == func_arguments[0])
                        {
                            current_entry = Zip_entries[i];
                            return;
                        }
                    }

                    Error_message(3, func_arguments[0]);
                }
                else if (func_arguments[0][0] == '.' && func_arguments[0][1] == '.' && func_arguments[0][2] == '/')
                {
                    if (current_entry->Up_folder != nullptr)
                    {
                        temp = current_entry->Up_folder;
                        temp->Current_entry_by_relative_path(func_arguments[0].substr(3), func_arguments[0]);
                        return;
                    }

                    Error_message(3, func_arguments[0]);
                }
                else if (func_arguments[0][0] == '.' && func_arguments[0][1] == '/')
                {
                    temp = current_entry;
                    temp->Current_entry_by_relative_path(func_arguments[0].substr(2), func_arguments[0]);
                    return;
                }
                else
                {
                    temp = current_entry;
                    if (temp->Down_entries.size() != 0)
                    {
                        temp->Current_entry_by_relative_path(func_arguments[0], func_arguments[0]);
                        return;
                    }

                    Error_message(3, func_arguments[0]);
                }

            }

        }
        else
        {
            Error_message(1, command);
            return;
        }
    }
    else
    {
        last_entry = current_entry;
        current_entry = Zip_entries[0];
        return;
    }
}

void clear(string command) //команда clear
{
    if (command.length() > 5)
    {
        Error_message(7, command);
    }
    else
    {
        system("cls");
    }
}

void uptime(string command) //команда uptime
{
    if (command.length() > 6)
    {
        Error_message(7, command);
    }
    else
    {
        time_t actual_time = time(nullptr);
        tm struct_time;
        localtime_s(&struct_time, &actual_time);
        if (struct_time.tm_min < 10)
        {
            cout << struct_time.tm_hour << ":0" << struct_time.tm_min << ":" << struct_time.tm_sec << " up ";
        }
        else if (struct_time.tm_sec < 10)
        {
            cout << struct_time.tm_hour << ":" << struct_time.tm_min << ":0" << struct_time.tm_sec << " up ";
        }
        else
        {
            cout << struct_time.tm_hour << ":" << struct_time.tm_min << ":" << struct_time.tm_sec << " up ";
        }

        long long sys_time_in_seconds = GetTickCount64() / 1000;
        int q_days = sys_time_in_seconds / (3600 * 24);
        int q_hours = (sys_time_in_seconds % (3600 * 24)) / 3600;
        int q_minutes = (sys_time_in_seconds % 3600) / 60;
        if (q_days == 1)
        {
            cout << q_days << "day, ";
        }
        else
        {
            cout << q_days << " days, ";
        }
        if (q_hours < 10)
        {
            cout << "0" << q_hours << ":";
        }
        else
        {
            cout << q_hours << ":";
        }
        if (q_minutes < 10)
        {
            cout << "0" << q_minutes << ", ";
        }
        else
        {
            cout << q_minutes << ", ";
        }

        int q_users = Count_users();
        if (q_users == 1)
        {
            cout << q_users << " user, ";
        }
        else
        {
            cout << q_users << " users, ";
        }

        double load_1_min = CPU_load();
        double load_5_min = (load_1_min + CPU_load()) / 2;
        double load_15_min = (load_1_min + load_5_min + CPU_load()) / 3;
        cout << "load average: " << fixed << setprecision(2) << load_1_min << ", " << load_5_min << ", " << load_15_min << endl;
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

Entry* Create_root_entry() //создание корневого каталога для VFS
{
    Entry* new_entry = new Entry;
    new_entry->name = "/";
    new_entry->path = "/";
    new_entry->is_folder = 1;
    new_entry->Up_folder = nullptr;
    return new_entry;
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
                    i = 0;

                    Zip_entries.push_back(Create_root_entry());

                    while (i < pos_centre_dir)
                    {
                        if (Zip_data[i] == 0x50 && Zip_data[i + 1] == 0x4b && Zip_data[i + 2] == 0x01 && Zip_data[i + 3] == 0x02)
                        {
                            int len_name = Zip_data[i + 28] | (Zip_data[i + 29] << 8); //сдвиг старшего байта вперёд, так как читается число аналогично сигнатуре справа налево

                            Entry* new_entry = new Entry;
                            new_entry->path = "";

                            for (int j = 0; j < len_name; j++)
                            {
                                new_entry->path = new_entry->path + (char)Zip_data[(i + 46) + j];
                            }


                            if (new_entry->path.back() != '/')
                            {
                                new_entry->is_folder = 0;
                            }
                            else
                            {
                                new_entry->path = new_entry->path.substr(0, new_entry->path.length() - 1);
                            }

                            new_entry->Rename_entry(0);
                        }
                        i++;
                    }

                    VFS_loaded = 1;
                    current_entry = Zip_entries[0];
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
            Invite(user_name, host_name);
            cout << command << endl;
            if (command == "exit")
            {
                break;
            }
            else if (command.substr(0, 2) == "ls")
            {
                if (VFS_loaded == 1)
                {
                    ls(command);
                }
                else
                {
                    Error_message(8, "");
                }
                continue;
            }
            else if (command.substr(0, 2) == "cd")
            {
                if (VFS_loaded == 1)
                {
                    cd(command);
                }
                else
                {
                    Error_message(8, "");
                }
                continue;
            }
            else if (command.substr(0, 5) == "clear")
            {
                clear(command);
                continue;
            }
            else if (command.substr(0, 6) == "uptime")
            {
                uptime(command);
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

    int q_arguments = func_arguments.size(), i = 0;
    for (int i = 0; i < func_arguments.size(); i++)
    {
        if (func_arguments[i] == "--vfs" && func_arguments.size() != i + 1 && Is_command(func_arguments[i + 1]) != true)
        {
            VFS(func_arguments[i + 1]);
            i++;
        }
        else if (func_arguments[i] == "--script" && func_arguments.size() != i + 1 && Is_command(func_arguments[i + 1]) != true)
        {
            Script(func_arguments[i + 1], user_name, host_name);
            i++;
        }
        else if (func_arguments[i] == "--config")
        {
            char func = 'a';
            if (func_arguments.size() == i + 1 || Is_command(func_arguments[i + 1]) == true)
            {
                Configuration_file("C:\\Users\\Света\\Desktop\\Configuration_management\\Practical_work_1\\Scripts and files\\Configuration.xml", func, user_name, host_name);
            }
            else
            {
                Configuration_file(func_arguments[i + 1], func, user_name, host_name);
                i++;
            }
        }
        else if ((func_arguments[i] == "--vfs" || func_arguments[i] == "--script") && (func_arguments.size() == i + 1 || Is_command(func_arguments[i + 1]) == true))
        {
            char func;

            if (func_arguments[0] == "--vfs")
            {
                func = 'v';
            }
            else
            {
                func = 's';
            }

            Configuration_file("C:\\Users\\Света\\Desktop\\Configuration_management\\Practical_work_1\\Scripts and files\\Configuration.xml", func, user_name, host_name);
        }
        else if (func_arguments[i] == "ls")
        {

            if (func_arguments.size() != i + 1 && Is_command(func_arguments[i + 1]) != true)
            {
                if (VFS_loaded == 0)
                {
                    Error_message(8, "");
                    continue;
                }
                string command = func_arguments[i] + " " + func_arguments[i + 1];
                ls(command);
                i++;
            }
            else
            {
                if (VFS_loaded == 0)
                {
                    Error_message(8, "");
                    continue;
                }
                ls(func_arguments[i]);
            }
        }
        else if (func_arguments[i] == "cd")
        {
            if (func_arguments.size() != i + 1 && Is_command(func_arguments[i + 1]) != true)
            {
                if (VFS_loaded == 0)
                {
                    Error_message(8, "");
                    continue;
                }
                string command = func_arguments[i] + " " + func_arguments[i + 1];
                cd(command);
                i++;
            }
            else
            {
                if (VFS_loaded == 0)
                {
                    Error_message(8, "");
                    continue;
                }
                cd(func_arguments[i]);
            }
        }
        else if (func_arguments[i] == "clear")
        {
            if (func_arguments.size() != i + 1 && Is_command(func_arguments[i + 1]) != true)
            {
                string command = func_arguments[i] + " " + func_arguments[i + 1];
                clear(command);
                i++;
            }
            else
            {
                clear(func_arguments[i]);
            }
        }
        else if (func_arguments[i] == "uptime")
        {
            if (func_arguments.size() != i + 1 && Is_command(func_arguments[i + 1]) != true)
            {
                string command = func_arguments[i] + " " + func_arguments[i + 1];
                uptime(command);
                i++;
            }
            else
            {
                uptime(func_arguments[i]);
            }
        }
        else
        {
            Error_message(1, func_arguments[i]);
            if (func_arguments.size() != i + 1 && Is_command(func_arguments[i + 1]) != true)
            {
                i++;
            }
        }
    }
}

void Launched_without_command(char* user_name, char* host_name) //запуск режима непосредственного диалога с пользователем
{
    string command;
    while (true)
    {
        Invite(user_name, host_name);
        command = Input();

        if (command == "exit")
        {
            break;
        }
        else if (command.substr(0, 2) == "ls")
        {
            if (VFS_loaded == 1)
            {
                ls(command);
                continue;
            }
            else
            {
                Error_message(8, "");
            }
            continue;
        }
        else if (command.substr(0, 2) == "cd")
        {
            if (VFS_loaded == 1)
            {
                cd(command);
                continue;
            }
            else
            {
                Error_message(8, "");
            }
            continue;
        }
        else if (command.substr(0, 5) == "clear")
        {
            clear(command);
            continue;
        }
        else if (command.substr(0, 6) == "uptime")
        {
            uptime(command);
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