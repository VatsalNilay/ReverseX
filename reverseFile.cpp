#include <sys/stat.h> //for mkdir()...
#include <sys/types.h> //for data types, eg. mode_t
#include <iostream> // std io for cpp
#include <fcntl.h> // for file control
#include <cstring> //converting string to readable human forms
#include <string>
#include<algorithm>
#include <cstdio> // for c io, esp(perror)
#include <unistd.h> // for sys calls


/*******************************************Resource Manager***************************************************** */
class ResourceManager {
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
        
        try {
            buffer = new char[size];
            buffer_size = size;
        } catch (const std::bad_alloc& e) {
            const char* fileError3 = "Error: No space left.\n";
            write(STDERR_FILENO, fileError3, strlen(fileError3));
            cleanup();
            _exit(1);
        }
        // std :: cout << "Allocatted Succefully.\n";
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
/******************************************Make Directory************************************************************** */
void make_dir(ResourceManager &rm)
{
    const char* dirName = "OutputFolder";
    mode_t mode = 0700;  // code for drwx------  permission
    int status = mkdir(dirName, mode);
    if(status == -1)
    {
        if(errno != EEXIST)   //error not equals to dir already existing
        {    
            perror("Error creating directory: \"OutputFolder\".");
            custom_exit(1,rm);
        }
    }
}
/********************************************Creating Dest file*************************************************************** */
int create_destination_file(const char* destination_path, const char* path, ResourceManager &rm)
{

    int dest_fd = open(destination_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (dest_fd == -1) {
        const char* fileError = "Error: Could not create destination file.\n";
        write(STDERR_FILENO, fileError, strlen(fileError));
        custom_exit(1,rm);
    }
    return dest_fd;
}
/************************************************Get output file name********************************************************************** */
std :: string getName(char *name)
{
    std :: string ans;
    int x = strlen(name) -1;
    while(x >= 0 and name[x] != '/')
    {
        ans += name[x];
        x--;
    }
    reverse(ans.begin(),ans.end());
    return ans;
}
/************************************************Progress********************************************************************** */
void update_progress(off_t totalProcessed, off_t fileSize, int& lastPercentage) 
{
    int percentage = (totalProcessed * 100) / fileSize;
    if (percentage != lastPercentage) {
        std::string progressStr = "\rProgress: " + std::to_string(percentage) + "%";
        write(STDOUT_FILENO, progressStr.c_str(), progressStr.length());
        lastPercentage = percentage;
    }
}


/************************************************File reversal by flag 0********************************************************************** */
void reverse_file_content_flag0(int src_fd, int dest_fd, ResourceManager& rm) {
    
    off_t fileSize = lseek(src_fd, 0, SEEK_END); // get file size
    if (fileSize == -1) {
        perror("Error opening source file.");
        close(dest_fd);
        custom_exit(1,rm);
    }

    off_t totalProcessed = 0;
    int lastPercentage = -1;

    off_t remainingBytes = fileSize;
    while (remainingBytes > 0) 
    {

        size_t currentChunkSize = (remainingBytes < rm.getBufferSize()) ? remainingBytes : rm.getBufferSize();

        if (lseek(src_fd, remainingBytes - currentChunkSize, SEEK_SET) == -1) 
        {
            perror("Error seeking in source file");
            close(dest_fd);
            custom_exit(1,rm);
        }

        ssize_t bytesRead = read(src_fd, rm.getBuffer(), currentChunkSize);
        if (bytesRead == -1) 
        {
            perror("Error reading from source file");
            close(dest_fd);
            custom_exit(1,rm);
        }

        for (ssize_t i = 0; i < bytesRead / 2; ++i) 
        {
            std::swap(rm.getBuffer()[i], rm.getBuffer()[bytesRead - 1 - i]);
        }

        
        ssize_t bytesWritten = write(dest_fd, rm.getBuffer(), bytesRead);
        if (bytesWritten != bytesRead) 
        {
            perror("Error writing to destination file");
            close(dest_fd);
            custom_exit(1,rm);
        }
        
        totalProcessed += bytesRead;
        update_progress(totalProcessed, fileSize, lastPercentage);

        remainingBytes -= currentChunkSize;
    }
    write(STDOUT_FILENO,"\n",1); //for newline after progress
    close(dest_fd); 
}


/************************************************File reversal by flag 1********************************************************************** */
void reverse_file_content_flag1(int src_fd, int dest_fd, ResourceManager& rm, int si, int ei) {
    off_t fileSize = lseek(src_fd, 0, SEEK_END);
    if (fileSize == -1) {
        perror("Error determining file size in source file.");
        close(dest_fd);
        custom_exit(1,rm);
    }

    if (si >= fileSize || ei >= fileSize) {
        const char* errorMsg4 = "Starting or ending index out of bounds.\n";
        write(STDERR_FILENO, errorMsg4, strlen(errorMsg4));
        close(dest_fd);
        custom_exit(1,rm);
    }
    off_t totalProcessed = 0;
    int lastPercentage = -1;

    // Process the first part (0 to si-1)
    off_t remainingBytes = si;
    while (remainingBytes > 0) 
    {
        size_t currentChunkSize = (remainingBytes < rm.getBufferSize()) ? remainingBytes : rm.getBufferSize();
        
        if (lseek(src_fd, remainingBytes - currentChunkSize, SEEK_SET) == -1) {
            perror("Error seeking in source file");
            close(dest_fd);
            custom_exit(1,rm);
        }

        ssize_t bytesRead = read(src_fd, rm.getBuffer(), currentChunkSize);
        if (bytesRead == -1) {
            perror("Error reading from source file");
            close(dest_fd);
            custom_exit(1,rm);
        }

        for (ssize_t i = 0; i < bytesRead / 2; ++i) {
            std::swap(rm.getBuffer()[i], rm.getBuffer()[bytesRead - 1 - i]);
        }

        if (write(dest_fd, rm.getBuffer(), bytesRead) != bytesRead) {
            perror("Error writing to destination file");
            close(dest_fd);
            custom_exit(1,rm);
        }
        //progress check1
        totalProcessed += currentChunkSize;
        update_progress(totalProcessed, fileSize, lastPercentage);

        remainingBytes -= currentChunkSize;
    }

    // Skip the section from si to ei and write it as-is
    off_t skipSize = ei - si + 1;
    if (lseek(src_fd, si, SEEK_SET) == -1) {
        perror("Error seeking to si in source file");
        close(dest_fd);
        custom_exit(1,rm);
    }

    while (skipSize > 0) {
        size_t currentChunkSize = (skipSize < rm.getBufferSize()) ? skipSize : rm.getBufferSize();
        ssize_t bytesRead = read(src_fd, rm.getBuffer(), currentChunkSize);
        if (bytesRead == -1) {
            perror("Error reading from source file");
            close(dest_fd);
            custom_exit(1,rm);
        }

        ssize_t bytesWritten = write(dest_fd, rm.getBuffer(), bytesRead);
        if (bytesWritten != bytesRead) {
            perror("Error writing to destination file");
            close(dest_fd);
            custom_exit(1,rm);
        }
        //progress check 2
        totalProcessed += currentChunkSize;
        update_progress(totalProcessed, fileSize, lastPercentage);
        skipSize -= currentChunkSize;
    }

    // Process the remaining part (ei+1 to end)
    remainingBytes = fileSize - (ei + 1);
    while (remainingBytes > 0) {
        size_t currentChunkSize = (remainingBytes < rm.getBufferSize()) ? remainingBytes : rm.getBufferSize();

        if (lseek(src_fd, fileSize - remainingBytes, SEEK_SET) == -1) {
            perror("Error seeking in source file");
            close(dest_fd);
            custom_exit(1,rm);
        }

        ssize_t bytesRead = read(src_fd, rm.getBuffer(), currentChunkSize);
        if (bytesRead == -1) {
            perror("Error reading from source file");
            close(dest_fd);
            custom_exit(1,rm);
        }

        for (ssize_t i = 0; i < bytesRead / 2; ++i) {
            std::swap(rm.getBuffer()[i], rm.getBuffer()[bytesRead - 1 - i]);
        }

        // Write the reversed chunk to the destination file
        if (write(dest_fd, rm.getBuffer(), bytesRead) != bytesRead) {
            perror("Error writing to destination file");
            close(dest_fd);
            custom_exit(1,rm);
        }
        //progress check 3
        totalProcessed += bytesRead;
        update_progress(totalProcessed, fileSize, lastPercentage);
        remainingBytes -= currentChunkSize;
    }
    write(STDOUT_FILENO,"\n",1); // for newline after progress
    close(dest_fd);
}
/************************************************Check if arg are integers or not********************************************************************** */

bool isInteger(const std::string& str) 
{
    if (str[0] == '-') return false;
    size_t start = 0;
    for (size_t i = start; i < str.size(); ++i) 
    {
        if (!std::isdigit(str[i])) return false;
    }
    return true;
}

/************************************************Main********************************************************************** */
int main(int argc, char * argv[])
{
    ResourceManager rm;
    if(argc != 3 and argc != 5)
    {
        // errno = EINVAL;
        const char* error_msg = "Error: Provide either 3 or 5 arguments.\n;";
        write(STDERR_FILENO, error_msg, strlen(error_msg)); //writing 'error_msg'    
        custom_exit(1,rm);
    }

    char *path = argv[1];
    const char *path2 = "./Assignment1/";

    if(argc == 3)
    {
        if(argv[2] != std :: to_string(0)) 
        {   
            if(argv[2] !=  std::to_string(1))
            {
                const char *errorArgs1 = "Error : Value of flag variable should either be 0 or 1.\n";
                write(STDERR_FILENO,errorArgs1, strlen(errorArgs1) );
            }
            else
            {
                const char *errorArgs1 = "Error : Need two more values with flag value as 1.\n";
                write(STDERR_FILENO,errorArgs1, strlen(errorArgs1) );
            }
            custom_exit(1,rm);
        }
        rm.allocate_resources(path, 1024*1024); //chunk size of 1 mb
        
        // Function to create directory
        make_dir(rm);
        //Create destination file
        std :: string destFile = getName(path);
        std :: string srcFile = std :: string(path2) + "0_" + destFile;
        const char* destFileName = srcFile.c_str();
        int dest_fd = create_destination_file(destFileName,path2, rm);
        off_t fileSize = lseek(rm.getfd(), 0, SEEK_END);
        if(fileSize == 0)
        {
            std::string progressStr = "\rProgress: " + std::to_string(100) + "%";
            write(STDOUT_FILENO, progressStr.c_str(), progressStr.length());
        }

        reverse_file_content_flag0(rm.getfd(), dest_fd, rm);
    }
    else
    {
        if(!isInteger(argv[3]) or !isInteger(argv[4]))
        {   
            const char* errorMsg4 = "Error: Starting and ending index should be non negative integers.\n";
            write(STDERR_FILENO, errorMsg4, strlen(errorMsg4));
            custom_exit(1,rm);

        }
        if (std::stoll(argv[3]) < 0 || std::stoll(argv[4]) < 0) {
            const char* errorMsg4 = "Error: Starting or ending index out of bounds.\n";
            write(STDERR_FILENO, errorMsg4, strlen(errorMsg4));
            custom_exit(1,rm);
        }

        if(argv[2] !=  std::to_string(1))
        {
            if(argv[2] !=  std::to_string(0))
            {
                const char *errorArgs2 = "Error : Value of flag variable should either be 0 or 1.\n";
                write(STDERR_FILENO,errorArgs2, strlen(errorArgs2) );
            }
            else
            {
                const char *errorArgs2 = "Error : No need to provide additional values with 0.\n";
                write(STDERR_FILENO,errorArgs2, strlen(errorArgs2) );
            }
            custom_exit(1,rm);
        }

        if(std::stoll(argv[3]) > std::stoll(argv[4]))
        {
            const char* errorArgs3 = "Error : Starting index can't be greater than ending index.\n";
            write(STDERR_FILENO,errorArgs3, strlen(errorArgs3));
            custom_exit(1,rm);
        }
        
        rm.allocate_resources(path, 1024*1024); //chunk size of 1 mb
        
        // Function to create directory
        make_dir(rm);

        //Create destination file
        std :: string destFile = getName(path);
        std :: string srcFile = std :: string(path2) + "1_" + destFile;
        const char* destFileName = srcFile.c_str();
        int dest_fd = create_destination_file(destFileName,path2, rm);

        reverse_file_content_flag1(rm.getfd(), dest_fd, rm,std :: stoi(argv[3]), std :: stoi(argv[4]));
        close(dest_fd);
    }
    custom_exit(0,rm);
}