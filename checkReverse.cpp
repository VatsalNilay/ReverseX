#include <sys/stat.h> //for mkdir()...
#include <sys/types.h> //for data types, eg. mode_t
#include <iostream> // std io for cpp
#include <fcntl.h> // for file control
#include <cstring> //converting string to readable human forms
#include <string>
#include <algorithm>
#include <cstdio> // for c io, esp(perror)
#include <unistd.h> // for sys call
#include <vector>


/*******************************************Resource Manager***************************************************** */
class ResourceManager 
{
private:

    int fd;    //for file descriptor
    char* buffer;  //for storing buffer values
    size_t buffer_size;  //for dynamic creation of buffer

public:
    ResourceManager() : fd(-1), buffer(nullptr), buffer_size(0){}
    
    ~ResourceManager() {
        cleanup();
    }

    int getfd(){return fd;}
    char* getBuffer() {return buffer;}
    size_t getBufferSize(){ return buffer_size;}
    
    void allocate_resources(const char* absolute_path, size_t size) {

        int e = strlen(absolute_path)-1;
        int s = e - 3;
        int a = 0;
        const char *extension = ".txt";
        for(int i = s; i < e; i++)
        {
            if(absolute_path[i] != extension[a++])
            {
                const char* filePathError = "Error : Input should be text file.\n";
                write(STDERR_FILENO, filePathError, strlen(filePathError));
                cleanup();
                _exit(1);
            }
        }
        
        fd = open(absolute_path, O_RDONLY);
        if (fd == -1) 
        {
            if (errno == EACCES) 
            {
                const char* permissionError = "Error: Read permission denied.\n";
                write(STDERR_FILENO, permissionError, strlen(permissionError));
            }
            else
            {
                const char*fileError2 = "Error: Provide correct path.\n";
                write(STDERR_FILENO, fileError2, strlen(fileError2));
            }
            cleanup();
            _exit(1);
        }

        try 
        {
            buffer = new char[size];
            buffer_size = size;
        } 
        catch (const std::bad_alloc& e) 
        {
            const char* fileError3 = "Error: No space left.\n";
            write(STDERR_FILENO, fileError3, strlen(fileError3));
            cleanup();
            _exit(1);
        }
    }

    void cleanup() {
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
        if (buffer != nullptr) {
            memset(buffer, 0, buffer_size);
            delete[] buffer;
            buffer = nullptr;
        }
    }
};

/*******************************************Custom Exit************************************************************* */
void __attribute__ ((noreturn)) custom_exit(int status, ResourceManager &rm)
{
    rm.cleanup();
    _exit(status);
}
/*********************************************Check Directory***************************************************************** */
bool check_dir(const char* dirPath)
{
    struct stat dirPathStat;
    bool ans;
    if (stat(dirPath, &dirPathStat) == 0) 
    {
        if (S_ISDIR(dirPathStat.st_mode)) 
        {
            const char* out1 = "Directory is created: Yes\n";
            write(STDOUT_FILENO, out1, strlen(out1));
            ans = true;
        }
        else
        {
            const char* out1 = "Directory is created: No\n";  //wrong file type
            write(STDOUT_FILENO, out1, strlen(out1));
            ans = false;
        }
    }
    else
    {
        const char* out1 = "Directory is created: No\n"; //non-existant path
        write(STDOUT_FILENO, out1, strlen(out1));
        ans = false;
    }
    return ans;
}
/*********************************************Comparing files***************************************************************** */
void compare(ResourceManager &rmOld,const char* oldFilePath, ResourceManager &rmNew, const char* newFilePath, bool x)
{

    if(!x)
    {
        const char* weAreNotTheSame = "Whether file contents are reversed in newfile: No\n";
        write(STDOUT_FILENO, weAreNotTheSame, strlen(weAreNotTheSame));   
        return;
    }

    int oldFileFd = rmOld.getfd();
    int newFileFd = rmNew.getfd();

    struct stat oldStat;
    if (stat(oldFilePath, &oldStat) == -1) 
    {
        const char* error2 = "Error: Can not open old file.\n";
        write(STDERR_FILENO,error2,strlen(error2));
        rmOld.cleanup();
        custom_exit(1,rmNew);
    }

    struct stat newStat;
    if (stat(newFilePath, &newStat) == -1) 
    {
        const char* error3 = "Error: Can not open new file.\n";
        write(STDERR_FILENO,error3,strlen(error3));
        rmOld.cleanup();
        custom_exit(1,rmNew);
    }
    off_t oldFileSize = oldStat.st_size;
    off_t newFileSize = newStat.st_size;
    off_t remainingSize = oldFileSize;
    bool flag = 1;
    while (remainingSize > 0) 
    {
        size_t currentChunkSize = remainingSize >= rmNew.getBufferSize() ? rmNew.getBufferSize() : remainingSize;

        off_t oldOffset = remainingSize - currentChunkSize;
        if (lseek(oldFileFd, oldOffset, SEEK_SET) == -1) 
        {
            const char* errorOld = "Error: Could not seek in old file.\n";
            write(STDERR_FILENO, errorOld, strlen(errorOld));   
            rmNew.cleanup();
            custom_exit(1,rmOld);
        }

        std::vector<char> oldChunk(currentChunkSize);
        if (read(oldFileFd, oldChunk.data(), currentChunkSize) != currentChunkSize) 
        {
            const char* errorOld2 = "Error: Could not read from old file.\n";
            write(STDERR_FILENO, errorOld2, strlen(errorOld2));   
            rmNew.cleanup();
            custom_exit(1,rmOld);
        }

        off_t newOffset = oldFileSize - remainingSize;
        if (lseek(newFileFd, newOffset, SEEK_SET) == -1) 
        {
            const char* errorNew = "Error: Could not seek in new file.\n";
            write(STDERR_FILENO, errorNew, strlen(errorNew));   
            rmNew.cleanup();
            custom_exit(1,rmOld);
        }

        std::vector<char> newChunk(currentChunkSize);
        if (read(newFileFd, newChunk.data(), currentChunkSize) != currentChunkSize) 
        {
            const char* errorNew2 = "Error: Could not read from new file.\n";
            write(STDERR_FILENO, errorNew2, strlen(errorNew2));   
            rmNew.cleanup();
            custom_exit(1,rmOld);
        }

        for (size_t i = 0; i < currentChunkSize; ++i) 
        {
            if (oldChunk[i] != newChunk[currentChunkSize - 1 - i]) 
            {
                flag = 0;
                break;    
            }
        }
        if(!flag)
            break;
        remainingSize -= currentChunkSize;
    }
    const char* reversed = "Whether file contents are reversed in newfile: Yes\n";
    const char* notReversed = "Whether file contents are reversed in newfile: No\n";
    if(!flag)
        write(STDOUT_FILENO, notReversed, strlen(notReversed));
    else
        write(STDOUT_FILENO, reversed, strlen(reversed));

}
/*********************************************Size Check**************************************************************** */
bool size_check(ResourceManager &rmOld, ResourceManager &rmNew)
{
    off_t fileSizeOld = lseek(rmOld.getfd(), 0, SEEK_END);
    off_t fileSizeNew = lseek(rmNew.getfd(),0,SEEK_END);
    bool ans;
    if(fileSizeNew == fileSizeOld)
        ans = true;
    else
        ans = false;
    return ans;
}
/*********************************************Check Permissions***************************************************************** */
void check_permission(std :: string name, const char* path, ResourceManager& rmOld, ResourceManager &rmNew)
{
    struct stat reqStat;
    if (stat(path, &reqStat) == -1) 
    {
        perror("Error");
        rmOld.cleanup();
        custom_exit(1,rmNew);
    }

    bool userRead    = reqStat.st_mode & S_IRUSR;
    std::string userR = "User has read permissions on " + name + (userRead ? ": Yes\n" : ": No\n");

    bool userWrite   = reqStat.st_mode & S_IWUSR;
    std::string userW = "User has write permissions on " + name + (userWrite ? ": Yes\n" : ": No\n");

    bool userExecute = reqStat.st_mode & S_IXUSR;
    std::string userX = "User has execute permissions on " + name + (userExecute ? ": Yes\n" : ": No\n");


    bool groupRead    = reqStat.st_mode & S_IRGRP;
    std::string groupR = "Group has read permissions on " + name + (groupRead ? ": Yes\n" : ": No\n");

    bool groupWrite   = reqStat.st_mode & S_IWGRP;
    std::string groupW = "Group has write permissions on " + name + (groupWrite ? ": Yes\n" : ": No\n");

    bool groupExecute = reqStat.st_mode & S_IXGRP;
    std::string groupX = "Group has execute permissions on " + name + (groupExecute ? ": Yes\n" : ": No\n");


    bool othersRead    = reqStat.st_mode & S_IROTH;
    std::string othersR = "Others have read permissions on " + name + (othersRead ? ": Yes\n" : ": No\n");

    bool othersWrite   = reqStat.st_mode & S_IWOTH;
    std::string othersW = "Others have write permissions on " + name + (othersWrite ? ": Yes\n" : ": No\n");

    bool othersExecute = reqStat.st_mode & S_IXOTH;
    std::string othersX = "Others have execute permissions on " + name + (othersExecute ? ": Yes\n" : ": No\n");


    write(STDOUT_FILENO, userR.c_str(), strlen(userR.c_str()));
    write(STDOUT_FILENO, userW.c_str(), strlen(userW.c_str()));
    write(STDOUT_FILENO, userX.c_str(), strlen(userX.c_str()));
    write(STDOUT_FILENO, groupR.c_str(), strlen(groupR.c_str()));
    write(STDOUT_FILENO, groupW.c_str(), strlen(groupW.c_str()));
    write(STDOUT_FILENO, groupX.c_str(), strlen(groupX.c_str()));
    write(STDOUT_FILENO, othersR.c_str(), strlen(othersR.c_str()));
    write(STDOUT_FILENO, othersW.c_str(), strlen(othersW.c_str()));
    write(STDOUT_FILENO, othersX.c_str(), strlen(othersX.c_str()));

}
/*********************************************Main***************************************************************** */
int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        const char* errorArg = "Error : Number of arguments provided should be 4.\n";
        write(STDERR_FILENO,errorArg,strlen(errorArg));
        _exit(1);
    }

    const char* newFilePath = argv[1];
    const char* oldFilePath = argv[2];
    const char* dirPath = argv[3];

    ResourceManager rmOld, rmNew;
    rmOld.allocate_resources(oldFilePath,1024*1024);
    rmNew.allocate_resources(newFilePath,1024*1024);

    bool y = check_dir(dirPath);

    bool x = size_check(rmOld,rmNew);
    compare(rmOld,oldFilePath, rmNew, newFilePath,x);

    if(x)
    {
        const char* out3 = "Both Files Sizes are Same : Yes\n"; 
        write(STDOUT_FILENO, out3, strlen(out3));
    }
    else
    {
        const char* out3 = "Both Files Sizes are Same : No\n"; 
        write(STDOUT_FILENO, out3, strlen(out3));
    }
    const char* newLine = "\n";
    write(STDOUT_FILENO, newLine, strlen(newLine));
    check_permission("newfile", newFilePath,rmOld,rmNew);

    write(STDOUT_FILENO, newLine, strlen(newLine));
    check_permission("oldfile", oldFilePath,rmOld, rmNew);

    write(STDOUT_FILENO, newLine, strlen(newLine));
    if(!y)
    {

    std::string userR = "User has read permissions on : No\n";
    std::string userW = "User has write permissions on : No\n";
    std::string userX = "User has execute permissions on : No\n";

    std::string groupR = "Group has read permissions on : No\n";
    std::string groupW = "Group has write permissions on : No\n";
    std::string groupX = "Group has execute permissions on : No\n";

    std::string othersR = "Others have read permissions on : No\n";
    std::string othersW = "Others have write permissions on : No\n";
    std::string othersX = "Others have execute permissions on : No\n";

        write(STDOUT_FILENO, userR.c_str(), strlen(userR.c_str()));
        write(STDOUT_FILENO, userW.c_str(), strlen(userW.c_str()));
        write(STDOUT_FILENO, userX.c_str(), strlen(userX.c_str()));
        write(STDOUT_FILENO, groupR.c_str(), strlen(groupR.c_str()));
        write(STDOUT_FILENO, groupW.c_str(), strlen(groupW.c_str()));
        write(STDOUT_FILENO, groupX.c_str(), strlen(groupX.c_str()));
        write(STDOUT_FILENO, othersR.c_str(), strlen(othersR.c_str()));
        write(STDOUT_FILENO, othersW.c_str(), strlen(othersW.c_str()));
        write(STDOUT_FILENO, othersX.c_str(), strlen(othersX.c_str()));
    }
    else
        check_permission("directory", dirPath, rmOld, rmNew);

    rmNew.cleanup();
    custom_exit(0,rmOld);

}