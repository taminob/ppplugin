function initialize()
    print("Lua initialize")
end

function loop(value)
    print("Lua: " .. tostring(value))
    return value * -1.5
end

print("Lua loading")

if not pcall(debug.getlocal, 4, 1) then
    print("Lua main")
end
