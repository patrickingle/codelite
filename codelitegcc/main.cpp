#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

std::string extract_file_name( const std::string& line );
char * normalize_path(const char * src, size_t src_len);

void * Memrchr(const void *buf, int c, size_t num);

#ifndef _WIN32
extern int ExecuteProcessUNIX(const std::string& commandline);
#else
extern int ExecuteProcessWIN(const std::string& commandline);
#endif

extern void WriteContent( const std::string& logfile, const std::string& filename, const std::string& flags );

// A thin wrapper around gcc
// Its soul purpose is to parse gcc's output and to store the parsed output
// in a sqlite database
int main(int argc, char **argv)
{
    // We require at least one argument
    if ( argc < 2 ) {
        return -1;
    }

    const char *pdb = getenv("CL_COMPILATION_DB");
    std::string commandline;
    for ( int i=1; i<argc; ++i ) {
        // Wrap all arguments with spaces with double quotes
        std::string arg = argv[i];

        // re-escape double quotes if needed
        size_t pos = arg.find('"');
        while ( pos != std::string::npos ) {
            arg.replace(pos, 1, "\\\""); // replace it with escapted slash
            pos = arg.find('"', pos + 2);
        }

        if ( arg.find(' ') != std::string::npos ) {
            std::string a = "\"";
            a += arg;
            a += '"';
            arg.swap(a);
        }
        commandline += arg + " ";
    }

    if ( pdb ) {
        std::string file_name = extract_file_name(commandline);
        if(file_name.empty() == false) {

#if __DEBUG
            printf("filename: %s\n", file_name.c_str());
#endif

            std::string logfile = pdb;
            logfile += ".txt";

            WriteContent(logfile, file_name, commandline);
        }
    }

    int exitCode = 0;
#ifdef _WIN32
    exitCode = ::ExecuteProcessWIN(commandline);
#else
    exitCode = ::ExecuteProcessUNIX(commandline);
#endif

    return exitCode;
}

size_t search_file_extension(const std::string &line, const std::string& ext)
{
    // note about searching the extension:
    // the 'line' passed to this function will *always* have a trailing space (see the 'main' function)
    // this fact makes the search easier for us:
    // always search for '{ext} ' or '{ext}"' this will avoid false matching like:
    // {source-name}.cpp.o
    
    std::vector<std::string> opts;
    opts.push_back(ext + " ");
    opts.push_back(ext + '"');
    
    for(size_t i=0; i<opts.size(); ++i) {
        size_t where = line.find(opts.at(i));
        if ( where != std::string::npos ) {
            return where;
        }
    }
    return std::string::npos;
}

std::string extract_file_name(const std::string& line)
{
    std::vector<std::string> extensions;
    extensions.push_back(".cpp");
    extensions.push_back(".cxx");
    extensions.push_back(".cc");
    extensions.push_back(".c");

    // locate any of the known c/cpp extensions
    size_t where = std::string::npos;

    std::string ext;
    for(size_t i=0; i<extensions.size(); ++i) {
        ext = extensions.at(i);
        where = search_file_extension(line, ext);
        if ( where != std::string::npos )
            break;
    }

    if ( where == std::string::npos )
        return "";

    std::string filename;
    if (line.at(where + ext.length()) == ' ') {
        // go backward until we find the first space
        size_t start = line.rfind(' ', where);
        if( start == std::string::npos )
            start = 0;
        filename = line.substr(start + 1, where + ext.length() - start);
    }

    if (line.at(where + ext.length()) == '"') {
        // go backward until we find the first space
        size_t start = line.rfind('"', where);
        if( start == std::string::npos )
            start = 0;
        filename = line.substr(start + 1, where + ext.length() - start);
    }

    std::replace(filename.begin(), filename.end(), '\\', '/');
    char* ret = normalize_path( filename.c_str(), filename.length() );
    filename = ret;
    free(ret);

#ifdef _WIN32
    std::replace(filename.begin(), filename.end(), '/', '\\');
#endif

    // trim whitespaces

    // rtrim
    filename.erase(0, filename.find_first_not_of("\t\r\v\n\" "));

    // ltrim
    filename.erase(filename.find_last_not_of("\t\r\v\n\" ")+1);

    return filename;
}

char * normalize_path(const char * src, size_t src_len)
{
    std::string strpath = src;
    size_t where = strpath.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    bool has_drive = (where == 0 && strpath.length() > 1 && strpath.at(1) == ':');

    char * res;
    size_t res_len;

    const char * ptr = src;
    const char * end = &src[src_len];
    const char * next;

    if (!has_drive && (src_len == 0 || src[0] != '/')) {

        // relative path

        char pwd[PATH_MAX];
        size_t pwd_len;

        if (getcwd(pwd, sizeof(pwd)) == NULL) {
            return NULL;
        }

        pwd_len = strlen(pwd);
        std::replace(pwd, pwd + pwd_len, '\\', '/');

        res = (char*)malloc(pwd_len + 1 + src_len + 1);
        memcpy(res, pwd, pwd_len);
        res_len = pwd_len;
    } else {
        res = (char*)malloc((src_len > 0 ? src_len : 1) + 1);
        res_len = 0;
    }

    for (ptr = src; ptr < end; ptr=next+1) {
        size_t len;
        next = (char*)memchr(ptr, '/', end-ptr);
        if (next == NULL) {
            next = end;
        }
        len = next-ptr;
        switch(len) {
        case 2:
            if (ptr[0] == '.' && ptr[1] == '.') {
                const char * slash = (char*)Memrchr(res, '/', res_len);
                if (slash != NULL) {
                    res_len = slash - res;
                }
                continue;
            }
            break;
        case 1:
            if (ptr[0] == '.') {
                continue;

            }
            break;
        case 0:
            continue;
        }

        if ( res_len == 0 && !has_drive )
            res[res_len++] = '/';
        else if ( res_len )
            res[res_len++] = '/';

        memcpy(&res[res_len], ptr, len);
        res_len += len;
    }

    if (res_len == 0) {
        res[res_len++] = '/';
    }
    res[res_len] = '\0';
    return res;
}

void * Memrchr(const void *buf, int c, size_t num)
{
    char *pMem = (char *) buf;

    for (;;) {
        if (num-- == 0) {
            return NULL;
        }

        if (pMem[num] == (unsigned char) c) {
            break;
        }

    }

    return (void *) (pMem + num);

}
