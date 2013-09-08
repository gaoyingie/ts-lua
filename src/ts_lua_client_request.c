
#include "ts_lua_util.h"

static int ts_lua_client_request_header_get(lua_State *L);
static int ts_lua_client_request_header_set(lua_State *L);
static int ts_lua_client_request_get_uri(lua_State *L);
static int ts_lua_client_request_set_uri(lua_State *L);
static int ts_lua_client_request_set_uri_args(lua_State *L);
static int ts_lua_client_request_get_uri_args(lua_State *L);

static void ts_lua_inject_client_request_header_api(lua_State *L);
static void tx_lua_inject_client_request_uri_api(lua_State *L);
static void tx_lua_inject_client_request_args_api(lua_State *L);


void
ts_lua_inject_client_request_api(lua_State *L)
{
    lua_newtable(L);

    ts_lua_inject_client_request_header_api(L);
    tx_lua_inject_client_request_uri_api(L);
    tx_lua_inject_client_request_args_api(L);

    lua_setfield(L, -2, "client_request");
}

static void
ts_lua_inject_client_request_header_api(lua_State *L)
{
    lua_newtable(L);         /* .header */

    lua_createtable(L, 0, 2);       /* metatable for .header */

    lua_pushcfunction(L, ts_lua_client_request_header_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, ts_lua_client_request_header_set);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2); 

    lua_setfield(L, -2, "header");
}

static int
ts_lua_client_request_header_get(lua_State *L)
{
    const char  *key;
    const char  *val;
    int         val_len;
    size_t      key_len;

    TSMLoc      field_loc;
    ts_lua_http_ctx  *http_ctx;

    http_ctx = ts_lua_get_http_ctx(L);

    /*  we skip the first argument that is the table */
    key = luaL_checklstring(L, 2, &key_len);

    if (key && key_len) {

        field_loc = TSMimeHdrFieldFind(http_ctx->client_request_bufp, http_ctx->client_request_hdrp, key, key_len);
        if (field_loc) {
            val = TSMimeHdrFieldValueStringGet(http_ctx->client_request_bufp, http_ctx->client_request_hdrp, field_loc, -1, &val_len);
            lua_pushlstring(L, val, val_len);
            TSHandleMLocRelease(http_ctx->client_request_bufp, http_ctx->client_request_hdrp, field_loc);

        } else {
            lua_pushnil(L);
        }

    } else {
        lua_pushnil(L);
    }

    return 1;
}

static int
ts_lua_client_request_header_set(lua_State *L)
{
    const char  *key;
    const char  *val;
    size_t      val_len;
    size_t      key_len;

    TSMLoc      field_loc;

    ts_lua_http_ctx  *http_ctx;

    http_ctx = ts_lua_get_http_ctx(L);

    /*   we skip the first argument that is the table */
    key = luaL_checklstring(L, 2, &key_len);
    val = luaL_checklstring(L, 3, &val_len);

    field_loc = TSMimeHdrFieldFind(http_ctx->client_request_bufp, http_ctx->client_request_hdrp, key, key_len);
    if (field_loc) {
        TSMimeHdrFieldValueStringSet(http_ctx->client_request_bufp, http_ctx->client_request_hdrp, field_loc, 0, val, val_len);
    } else if (TSMimeHdrFieldCreateNamed(http_ctx->client_request_bufp, http_ctx->client_request_hdrp,
                key, key_len, &field_loc) != TS_SUCCESS) {
        return 0;
    } else {
        TSMimeHdrFieldValueStringSet(http_ctx->client_request_bufp, http_ctx->client_request_hdrp, field_loc, -1, val, val_len);
        TSMimeHdrFieldAppend(http_ctx->client_request_bufp, http_ctx->client_request_hdrp, field_loc);
    }

    return 0;
}

static void
tx_lua_inject_client_request_uri_api(lua_State *L)
{
    lua_pushcfunction(L, ts_lua_client_request_set_uri);
    lua_setfield(L, -2, "set_uri");

    lua_pushcfunction(L, ts_lua_client_request_get_uri);
    lua_setfield(L, -2, "get_uri");
}


static int
ts_lua_client_request_get_uri(lua_State *L)
{
    char        uri[TS_LUA_MAX_URL_LENGTH];
    const char  *path;
    int         path_len;
    int         uri_len;

    ts_lua_http_ctx  *http_ctx;

    http_ctx = ts_lua_get_http_ctx(L);

    path = TSUrlPathGet(http_ctx->client_request_bufp, http_ctx->client_request_url, &path_len);

    uri_len = snprintf(uri, TS_LUA_MAX_URL_LENGTH, "/%.*s", path_len, path);

    lua_pushlstring(L, uri, uri_len);

    return 1;
}

static int
ts_lua_client_request_set_uri(lua_State *L)
{
    return 0;
}


static void
tx_lua_inject_client_request_args_api(lua_State *L)
{
    lua_pushcfunction(L, ts_lua_client_request_set_uri_args);
    lua_setfield(L, -2, "set_uri_args");

    lua_pushcfunction(L, ts_lua_client_request_get_uri_args);
    lua_setfield(L, -2, "get_uri_args");
}

static int
ts_lua_client_request_get_uri_args(lua_State *L)
{
    const char  *param;
    int         param_len;

    ts_lua_http_ctx  *http_ctx;

    http_ctx = ts_lua_get_http_ctx(L);

    param = TSUrlHttpQueryGet(http_ctx->client_request_bufp, http_ctx->client_request_url, &param_len);

    if (param && param_len > 0) {
        lua_pushlstring(L, param, param_len);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

static int
ts_lua_client_request_set_uri_args(lua_State *L)
{
    const char  *param;
    size_t      param_len;

    ts_lua_http_ctx  *http_ctx;

    http_ctx = ts_lua_get_http_ctx(L);

    param = luaL_checklstring(L, 1, &param_len);
    TSUrlHttpQuerySet(http_ctx->client_request_bufp, http_ctx->client_request_url, param, param_len);

    return 0;
}

