function accept_number_string_bool(n, s, b)
    local is_number = type(n) == "number"
    local is_string = type(s) == "string"
    local is_bool = type(b) == "boolean"

    return is_number and is_string and is_bool
end

function serialize_array(t)
    local result = ""
    table.sort(t)
    for i, v in ipairs(t) do
        result = result .. tostring(i) .. ":" .. tostring(v) .. ","
    end
    return result
end

function access_table(t, key)
    return t[key]
end

function return_array()
    return { "a", "b", "c" }
end

function return_map()
    return {
        a = 1,
        b = 2,
        c = 3,
        d = 4,
    }
end

function return_empty_table()
    return {}
end

function identity(x)
    return x
end
