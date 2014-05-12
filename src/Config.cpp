#include <map>
#include <string>
#include <ini.h>

#include "Common.h"
#include "Lua.h"
#include "Config.h"


using namespace std;


map<string,string> g_ConfigValues;

int IniEntryCallback( void* user, const char* section, const char* name, const char* value );

bool InitConfig( const int argc, char** argv )
{
    // Try to load config.ini in working directory ...
    ini_parse("config.ini", IniEntryCallback, NULL);


    // Parse command line arguments ...
    for(int i = 1; i < argc; ++i)
    {
        const string arg = argv[i];
        // --key=value

        if(
            arg.size() < 2 ||
            arg[0] != '-' ||
            arg[1] != '-'
        )
        {
            Log("Bad argument '%s'", arg.c_str());
            break;
        }

        const size_t equalsPos = arg.find('=');

        if(equalsPos == string::npos)
        {
            Log("Bad argument '%s'", arg.c_str());
            break;
        }

        string key   = arg.substr(2, equalsPos-2);
        string value = arg.substr(equalsPos+1);

        if(key == "config")
        {
            Log("Reading config file %s ..", value.c_str());
            ini_parse(value.c_str(), IniEntryCallback, NULL);
        }
        else
        {
            Log("%s = %s", key.c_str(), value.c_str());
            g_ConfigValues[key] = value;
        }
    }

    return true;
}

void DestroyConfig()
{
    g_ConfigValues.clear();
}

const char* GetConfigString( const char* key, const char* defaultValue )
{
    const map<string,string>::const_iterator i =
        g_ConfigValues.find(key);

    if(i != g_ConfigValues.end())
        return i->second.c_str();
    else
        return defaultValue;
}

int GetConfigInt( const char* key, int defaultValue )
{
    const char* str = GetConfigString(key, NULL);
    return str==NULL ? defaultValue : atoi(str);
}

float GetConfigFloat( const char* key, float defaultValue )
{
    const char* str = GetConfigString(key, NULL);
    return str==NULL ? defaultValue : atof(str);
}

bool GetConfigBool( const char* key, bool defaultValue )
{
    const char* str = GetConfigString(key, NULL);
    if(str == NULL)
        return defaultValue;

    switch(str[0])
    {
        case '0':
        case 'n':
        case 'N':
        case 'f':
        case 'F':
            return false;

        case '1':
        case 'y':
        case 'Y':
        case 't':
        case 'T':
            return true;

        default:
            return defaultValue;
    }
}

int IniEntryCallback( void* user, const char* section, const char* name, const char* value )
{
    using namespace std;

    string key;
    if(section == NULL)
        key = string(name);
    else
        key = string(section) + string(".") + string(name);

    Log("%s = %s", key.c_str(), value);
    g_ConfigValues[key] = value;
    return 1;
}

// --- lua bindings ---

int Lua_GetConfigValue( lua_State* l )
{
    const char* key = luaL_checkstring(l, 1);

    const char* stringValue = GetConfigString(key, NULL);
    if(stringValue == NULL)
    {
        lua_pushvalue(l, 2); // push default value
        return 1;
    }

    switch(lua_type(l, 2)) // use type of default value to determine return type
    {
        case LUA_TSTRING:
            lua_pushstring(l, stringValue);
            return 1;

        case LUA_TNUMBER:
            lua_pushnumber(l, GetConfigFloat(key, 0));
            return 1;

        case LUA_TBOOLEAN:
            lua_pushboolean(l, GetConfigBool(key, false));
            return 1;

        default:
            luaL_error(l, "Unsupported type. (Only string, number and bool!)");
            return 0;
    }
}

AutoRegisterInLua()
{
    return
        RegisterFunctionInLua("GetConfigValue", Lua_GetConfigValue);
}
