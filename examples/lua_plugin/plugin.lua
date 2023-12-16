require 'os'

function initialize()
	print("initialize")
end

function loop(something)
	while true do
		local start = os.time()
		print("loop: " .. something)
		while start + 1 > os.time() do
		end
		-- TODO: execute('sleep 1') blocks SIGINT signal, would have to check return value
	end
end

print("loading")
