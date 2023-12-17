function initialize()
    print("Lua initialize")
end

function loop(value)
    print("Lua: " .. tostring(value))
    return value * -1.5
end
