string_global = "abc"
int_global = 12
float_global = 42.0
bool_global = false
char_global = "9"
dict_global = { a = {}, b = { f = false }, c = { t = true } }
list_global = { {}, { 0 }, { 0, false } }

function get_global(name)
    return _G[name]
end

function accept_int_string_bool_float(i, s, b, f)
    local is_integer = type(i) == "number"
    local is_float = type(f) == "number"
    local is_string = type(s) == "string"
    local is_bool = type(b) == "boolean"

    return is_integer and is_float and is_string and is_bool
end

function accept_list(t)
    local result = ""
    table.sort(t)
    for _, v in ipairs(t) do
        result = result .. tostring(v) .. ","
    end
    return result
end

function accept_dict(t)
    t["new"] = "new value"
    return t
end

function identity(x)
    return x
end
