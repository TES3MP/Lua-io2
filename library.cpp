#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cmath>

#define SOL_CHECK_ARGUMENTS 1

#include <sol.hpp>
#include "myperms.hpp"
#include "library.h"

#if HAVE_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

using namespace std;

#ifdef _WIN32
#define mystring wstring
#else
#define mystring string
#endif

namespace io2
{
    inline string wstringToString(const mystring &str)
    {
#ifndef _WIN32
        return str;
#else
        return wstring_convert<codecvt_utf8_utf16<mystring::value_type>, mystring::value_type>().to_bytes(str);
#endif
    }

    class io
    {
    public:

        explicit io(const mystring &file) : io(file, "r") {}

        explicit io(const mystring &file, const string &_mode) //should be [rwa][+]?[b]?
        {
            ios_base::openmode mode = ios_base::in;
            if (_mode[0] == 'w') // Truncate file to zero length or create text file for writing
                mode = ios_base::out | ios_base::trunc;
            else if (_mode[0] == 'a') //  Open for appending (writing at end of file).
                mode = ios_base::app; //  The file is created if it does not exist

            if (_mode.size() > 1)
            {
                if (_mode[1] == '+')
                {
                    mode |= ios_base::in | ios_base::out;
                    if (_mode.size() > 2) // [rw]+b
                        mode |= ios_base::binary;
                }
                else if (_mode[1] == 'b')
                    mode |= ios_base::binary; // binary mode
            }
            stream.open(file, mode);
            if (!stream.is_open())
            {
                string err = "io2.file.new(): Cannot open ";
                err += wstringToString(file);
                err += " in mode \"";
                err += !_mode.empty() ? _mode : "r";
                err += "\"";
                throw runtime_error(err);
            }
        };

        void close()
        {
            stream.close();
        }

        lua_Number readnumber()
        {
            lua_Number val = 0;
            stream >> val;
            return val;
        }

        std::string readline()
        {
            string line;
            getline(stream, line);
            return line;
        }

        std::string readbytes(size_t bytes)
        {
            vector<string::value_type> buff(bytes);

            stream.read(&buff[0], bytes);
            return &buff[0];
        }

        sol::variadic_results read(sol::variadic_args va, sol::this_state s)
        {
            sol::state_view lua(s);
            sol::variadic_results vares;
            for (auto &&arg : va)
            {
                if (arg.is<string>())
                {
                    string str = arg.as<string>();
                    if (str == "*all" || str == "*a")
                    {
                        auto _size = static_cast<size_t>(size());
                        string _str(readbytes(_size), 0, _size);
                        vares.push_back({lua, sol::in_place, _str});
                    }
                    else if (str == "*l")
                        vares.push_back({lua, sol::in_place, readline()});
                    else if (str == "*n")
                        vares.push_back({lua, sol::in_place, readnumber()});
                }
                else if (arg.is<lua_Integer>())
                {
                    auto n = arg.as<lua_Integer>();
                    string str(readbytes(n), 0, n);
                    if (stream.eof())
                        vares.push_back({lua, sol::in_place, sol::nil});
                    vares.push_back({lua, sol::in_place, str});
                }
            }

            return vares;
        }

        void writeln(const std::string &str)
        {
            stream << str << endl;
        }

        sol::function lines(sol::this_state s)
        {
            sol::state_view lua(s);

            return sol::object(lua, sol::in_place, [this](sol::this_state _s) {
                sol::state_view _lua(_s);
                string line;
                getline(stream, line);
                if (line.empty())
                    return sol::make_reference(_lua, sol::nil);
                return sol::make_reference(_lua, line);
            });
        }

        void write(sol::variadic_args va)
        {
            for (auto &&var : va)
            {
                if (var.is<string>())
                    stream << var.as<string>();
                else if (var.is<lua_Integer>())
                    stream << var.as<lua_Integer>();
                else if (var.is<lua_Number>())
                    stream << var.as<lua_Number>();
                else
                    cerr << "io2.file:write(): Unsupported type." << endl;
            }
        }

        void writebyte(unsigned char byte)
        {
            stream.write((char*) &byte, 1);
        }

        streamoff size()
        {
            auto curPos = stream.tellg();

            stream.seekg(0, ios_base::end);
            auto size = stream.tellg();
            stream.seekg(curPos);
            return size;
        }
        void seek(const string &whence, streamoff offset)
        {
            if (whence == "cur")
                stream.seekg(offset, ios_base::cur);
            else if (whence == "set")
                stream.seekg(offset, ios_base::beg);
            else if (whence == "end")
                stream.seekg(offset, ios_base::end);
        }

        streamoff pos()
        {
            return stream.tellg();
        }
    private:
        fstream stream;
    };

    sol::table fs(sol::state_view lua)
    {
        sol::table module = lua.create_table();

        module.set_function("mkdir", [](const mystring &dir) {
            fs::create_directory(dir);
        });

        module.set_function("rm", [](const mystring &path) {
            fs::remove(path);
        });

        module.set_function("rmall", [](const mystring &path) {
            fs::remove_all(path);
        });

        module.set_function("chdir", [](const mystring &path) {
            fs::current_path(path);
        });

        module.set_function("currentdir", [](){
            return fs::current_path().c_str();
        });

        module.set_function("dir", [](sol::optional<mystring> maybePath, sol::this_state L) -> sol::object {
            fs::path path;
            if (maybePath)
                path = maybePath.value();
            else
                path = fs::current_path();

            sol::state_view lua(L);
            auto it= make_shared<fs::directory_iterator>(path);
            return sol::object(lua, sol::in_place, [path, it](sol::this_state _L) -> sol::object {
                sol::state_view _lua(_L);
                static fs::directory_iterator endIt;

                if ((*it.get()) == endIt)
                    return sol::object(_lua, sol::in_place, sol::nil);
                string str = (*it)->path().filename().string();
                ++(*it);
                return sol::object(_lua, sol::in_place, str);
            });
        });

        module.set_function("isdir", [](const mystring & pathStr) {
            return fs::is_directory(pathStr);
        });

        module.set_function("permissions", [](const mystring & pathStr, sol::optional<sol::object> maybePerm) {
            auto getPermsOfFile = [](const mystring & file) {
                myfs::perms p = (myfs::perms) fs::status(file).permissions();
                stringstream sstr;
                sstr << ((p & myfs::perms::owner_read) != myfs::perms::none ? "r" : "-")
                          << ((p & myfs::perms::owner_write) != myfs::perms::none ? "w" : "-")
                          << ((p & myfs::perms::owner_exec) != myfs::perms::none ? "x" : "-")
                          << ((p & myfs::perms::group_read) != myfs::perms::none ? "r" : "-")
                          << ((p & myfs::perms::group_write) != myfs::perms::none ? "w" : "-")
                          << ((p & myfs::perms::group_exec) != myfs::perms::none ? "x" : "-")
                          << ((p & myfs::perms::others_read) != myfs::perms::none ? "r" : "-")
                          << ((p & myfs::perms::others_write) != myfs::perms::none ? "w" : "-")
                          << ((p & myfs::perms::others_exec) != myfs::perms::none ? "x" : "-");
                return sstr.str();
            };

            auto getPerms = [](string perms) {
                myfs::perms p = myfs::perms::none;
                if (perms.size() != 9)
                {
                    cerr << "io2.fs.permissions(): incorrect permissions" << endl;
                    return (fs::perms) p;
                }

                p |= perms[0] == 'r' ?  myfs::perms::owner_read : myfs::perms::none;
                p |= perms[1] == 'w' ?  myfs::perms::owner_write : myfs::perms::none;
                p |= perms[2] == 'x' ?  myfs::perms::owner_exec : myfs::perms::none;
                p |= perms[3] == 'r' ?  myfs::perms::group_read : myfs::perms::none;
                p |= perms[4] == 'w' ?  myfs::perms::group_write : myfs::perms::none;
                p |= perms[5] == 'x' ?  myfs::perms::group_exec : myfs::perms::none;
                p |= perms[6] == 'r' ?  myfs::perms::others_read : myfs::perms::none;
                p |= perms[7] == 'w' ?  myfs::perms::others_write : myfs::perms::none;
                p |= perms[8] == 'x' ?  myfs::perms::others_exec : myfs::perms::none;
                return (fs::perms) p;
            };

            if (maybePerm)
            {
                sol::object obj = maybePerm.value();
                if (obj.is<unsigned>())
                {
                    unsigned val = obj.as<unsigned>();

                    auto octToDec = [](unsigned octal) {
                        int remainder, decimal = 0;
                        for (int i = 0; octal != 0; ++i)
                        {
                            remainder = octal % 10;
                            octal /= 10;
                            decimal += remainder * (int) pow(8, i);
                        }
                        return decimal;
                    };

                    fs::permissions(pathStr, (fs::perms) octToDec(val));
                }
                else if (obj.is<string>())
                    fs::permissions(pathStr, getPerms(obj.as<string>()));
                else
                {
                    cerr << "io2.fs.permissions(): incorrect type of permissions" << endl;
                    return string();
                }
            }

            return getPermsOfFile(pathStr);
        });

        return module;
    }

    sol::table io2(sol::this_state L)
    {
        sol::state_view lua(L);
        sol::table module = lua.create_table();
        module.new_usertype<io>("file",
                                "close", &io::close,
                                "size", &io::size,
                                "seek", &io::seek,
                                "pos", &io::pos,
                                "readnumber", &io::readnumber,
                                "readline", &io::readline,
                                "readbytes", &io::readbytes,
                                "read", &io::read,
                                "write", &io::write,
                                "writeln", &io::writeln,
                                "lines", &io::lines,
                                "writebyte", &io::writebyte
        );

        module.set_function("open", [](const string &file, sol::object mode, sol::this_state L) ->sol::object {
            try
            {
                mystring tmp;
#ifdef _WIN32
                tmp = wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t>().from_bytes(file);
#else
                tmp = file;
#endif
                shared_ptr<io> ioPtr;
                if (mode.valid() && mode.is<string>())
                    ioPtr = make_shared<io>(tmp, mode.as<string>());
                else
                    ioPtr = make_shared<io>(tmp);

                return sol::make_object(sol::state_view(L), ioPtr);
            }
            catch (exception &e)
            {
                cerr << "io2.open(): " << e.what() << endl;
                return sol::make_object(sol::state_view(L), sol::nil);
            }
        });

        module.set_function("close", [](shared_ptr<io> io) {
            io->close();
        });

        module["fs"] = fs(lua);

        return module;
    }
}


extern "C" int luaopen_io2(lua_State *L)
{
    return sol::stack::call_lua(L, 1, io2::io2);
}
