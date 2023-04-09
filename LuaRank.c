#include <stdlib.h>
#include <stdint.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "Rank.h"

static int 
LuaRankFree(lua_State *L) {
    Rank *rank = (Rank *)lua_touserdata(L, 1);
    RankFree(rank);
    return 0;
}

static int 
LuaRankNew(lua_State *L) {
    uint32_t compareNum = luaL_checkinteger(L, 1);
    uint8_t compareSet[compareNum];
    for (int i = 0; i < compareNum; ++i) {
        compareSet[i] = luaL_checkinteger(L, i + 2);
    }
    Rank *rank = RankNew(compareSet, compareNum);
    lua_pushlightuserdata(L, rank);
    luaL_getmetatable(L, "metaRank");
    lua_setmetatable(L, -2);
    return 1;
}

static int 
LuaRankSort(lua_State *L) {
    Rank *rank = (Rank *)lua_touserdata(L, 1);
    RankSort(rank);
    return 0;
}

static int 
LuaRankLength(lua_State *L) {
    Rank *rank = (Rank *)lua_touserdata(L, 1);
    uint32_t len = RankLength(rank);
    lua_pushinteger(L, len); 
    return 1;
}

static int 
LuaRankGetPosByUid(lua_State *L) {
    Rank *rank = (Rank *)lua_touserdata(L, 1);
    uint32_t uid = luaL_checkinteger(L, 2);
    uint32_t rankPos = RankGetPosByUid(rank, uid);
    if (rankPos == -1){
        return 0;
    }
    lua_pushinteger(L, rankPos);
    return 1;
}

static int 
LuaRankGetElementByUid(lua_State *L) {
    Rank *rank = (Rank *)lua_touserdata(L, 1);
    uint32_t uid = luaL_checkinteger(L, 2);
    uint32_t rankPos = 0;
    DATA_TYPE data[rank->compareNum];

    if (RankGetElementByUid(rank, uid, &rankPos, data) == -1){
        return 0;
    }

    lua_pushinteger(L, rankPos);
    lua_newtable(L);
    for (int i = 0; i < rank->compareNum; ++i){
        lua_pushinteger(L, data[i]);
        lua_rawseti(L, -2, i+1);
    }
    return 2;
}

static int 
LuaRankForeachByRange(lua_State *L) {
    Rank *rank = (Rank *)lua_touserdata(L, 1);
    uint32_t left = luaL_checkinteger(L, 2);
    uint32_t right = luaL_checkinteger(L, 3);
    luaL_checktype(L, 4, LUA_TFUNCTION);

    if (left < 1 || left > right){
        luaL_error(L, "Rank range[%d,%d] error!", left, right);
        return 0;
    }

    RankSort(rank);

    if (right > RankLength(rank)){
        right = RankLength(rank);
    }

    int ret = 0;
    for (int i = left - 1; i < right; ++i) {
        Element *element = rank->arr[i];
        lua_pushvalue(L, 4);
        lua_pushinteger(L, element->uid);
        for (int j = 0; j < rank->compareNum; ++j) {
            lua_pushinteger(L, element->data[j]);
        }
        if (lua_pcall(L, rank->compareNum+1, 1, 0) != LUA_OK) {
            luaL_error(L, lua_tostring(L, -1));
            lua_pop(L, 1);
            continue;
        }
        if (!lua_isboolean(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        ret = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (ret) {
            break;
        }
    }
    return 0;
}

static int 
LuaRankInsertOrUpdate(lua_State *L) {
    Rank *rank = (Rank *)lua_touserdata(L, 1);
    uint32_t uid = luaL_checkinteger(L, 2);
    DATA_TYPE data[rank->compareNum];
    for (int i = 0; i < rank->compareNum; ++i) {
        data[i] = luaL_checkinteger(L, i+3);
    }
    RankInsertOrUpdateByUid(rank, uid, data);
    return 0;
}

int 
luaopen_RankCore(lua_State *L) {
    luaL_checkversion(L);

    luaL_Reg RankFunc[] = {
        {"New", LuaRankNew},
        { NULL, NULL },
    };

    luaL_Reg RankObjFunc[] = {
        {"Update", LuaRankInsertOrUpdate},
        {"Sort", LuaRankSort},
        {"Length", LuaRankLength},
        {"QueryPos", LuaRankGetPosByUid},
        {"QueryData", LuaRankGetElementByUid},
        {"ForeachByRange", LuaRankForeachByRange},
        {"__gc", LuaRankFree},
        {"__close", LuaRankFree},
        { NULL, NULL },
    };

    luaL_newmetatable(L, "metaRank");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, RankObjFunc, 0);

    lua_createtable(L, 0, sizeof(RankFunc)/sizeof(RankFunc[0]));
    luaL_setfuncs(L, RankFunc, 0);
    return 1;
}