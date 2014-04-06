#ifndef __APOAPSIS_LUA__
#define __APOAPSIS_LUA__

extern "C"
{
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>
}

/* --- Tipps ---

- Zugriff auf lokale Variablen ist schneller als auf Globale.
  Das liegt daran, dass globale Variablen immer in einer Hashmap nachgeschaut werden müssen.
  Lokale Variablen sind dagegen in VM Registern, welche ledeglich über einen Index angesprochen werden.

- Speicherallokalisierung ist langsam.
  Das ist aber überall so.
  Also nach Möglichkeit vorher allokalisieren.

- In C lua_raw* in kritischen Bereichen nutzen.
  Normalerweise wird bei Lookups der Metatable beachtet. lua_raw* ignoriert diesen.

- String concatenation (a..b oder strsub() usw) ist langsam.
  Strings sind in Lua immer einzigartig. D.h. immer wenn ein neuer String kreiert wird,
  muss dieser in einem internen Table abgeglichen werden.

- Sprachkonstrukte sind schneller als Funktionen.
  Z.B. statt ipairs 'for k=1, #tbl do local v = tbl[k];'

- NUR DORT OPTIMIEREN WO ES AUCH NOTWENDIG IST!

- Strings als Enums nutzen.
*/

/* -------
luaL_check[any,int,string,number,...] - check if stack element n has the type t and returns it
luaL_checkoption - check and convert string to enum
luaL_error - raises an error (return luaL_error(...))
------- */

bool InitLua();
void DestroyLua();

lua_State* GetLuaState();

void UpdateLua();

void RegisterFunctionInLua( const char* name, lua_CFunction fn );

/**
 * Registers a user data type by name.
 *
 * User data created by ::PushUserDataToLua is annotated with type information,
 * so #GetUserDataFromLua can assure that the element has the correct type.
 *
 * @param gcCallback
 * Function that is called before Lua frees garbage collected user data.
 * May be `NULL` - in this case Lua doesn't notify you at all.
 */
void RegisterUserDataTypeInLua( const char* name, lua_CFunction gcCallback );

/**
 * Allocates memory for a user defined data structure in Lua
 * and pushes it on top of the stack.
 *
 * @param size
 * Data size in bytes.
 *
 * Since user data of the same type can still have different sizes,
 * it's your responsibillity to ensure correct data access.
 *
 * @return
 * The allocated, but unintialized, user data
 * or `NULL` if the type does not exist.
 */
void* PushUserDataToLua( lua_State* l, const char* typeName, int size );

/**
 * Checks if the element at stackPosition has the requested user data type
 * and retrieves it.
 *
 * @return
 * Pointer to the user data or `NULL` if the type does not match.
 *
 * @note
 * Alternatively ::luaL_checkudata can be used to retrieve and check userdata.
 * It raises an Lua exception if the userdata doesn't match the given type.
 */
void* GetUserDataFromLua( lua_State* l, int stackPosition, const char* typeName );

/**
 * Registers a new event and returns its id.
 *
 * @note
 * An event may only be registered once.
 *
 * @return
 * The events id.
 */
int RegisterLuaEvent( const char* name );

/**
 * Runs the script callback associated with the given event.
 *
 * @note
 * If no callback was set for the event,
 * it will behave as if an empty function was called.
 *
 * @param id
 * Event id returned by ::RegisterLuaEvent.
 *
 * @param argumentCount
 * Number of elements in the lua stack, that shall be used
 * as function parameters for the callback.
 *
 * @param pushReturnValues
 * Whether to push values returned by the callback in the stack.
 *
 * @return
 * Number of values that were pushed in the stack or 0.
 */
int FireLuaEvent( lua_State* l, int id, int argumentCount, bool pushReturnValues );

#endif
