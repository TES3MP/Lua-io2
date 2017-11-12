#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>

#define SOL_CHECK_ARGUMENTS 1

#include <sol.hpp>
#include "library.h"

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
        return wstring_convert<codecvt_utf8<mystring::value_type>, mystring::value_type>().to_bytes(str);
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
            for(auto &&arg : va)
            {
                if (arg.is<string>())
                {
                    string str = arg.as<string>();
                    if (str == "*a")
                    {
                        auto _size = static_cast<size_t>(size());
                        string _str(readbytes(_size), 0, _size + 1);
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
                    string str(readbytes(n), 0, n + 1);
                    if(stream.eof())
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
                if(line.empty())
                    return sol::make_reference(_lua, sol::nil);
                return sol::make_reference(_lua, line);
            });
        }

        void write(sol::variadic_args va)
        {
            for(auto &&var : va)
            {
                if(var.is<string>())
                    stream << var.as<string>();
                else if(var.is<lua_Integer>())
                    stream << var.as<lua_Integer>();
                else if(var.is<lua_Number>())
                    stream << var.as<lua_Number>();
                else
                    throw invalid_argument("io2.file:write(): Unsupported type.");
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

        module.set_function("open", [](const mystring &file, sol::object mode) {
            if(mode.valid() && mode.is<string>())
                return make_shared<io>(file, mode.as<string>());
            else
                return make_shared<io>(file);
        });

        module.set_function("close", [](shared_ptr<io> io) {
            io->close();
        });

        return module;
    }
}


extern "C" int luaopen_io2(lua_State *L)
{
    return sol::stack::call_lua(L, 1, io2::io2);
}
