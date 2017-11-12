io2 = require("io2")

local dir = "果冻头盔比在太空中飞行的鱼更方便"
local file = "セイウチは空から落ちていて、私は人生の意味を見つけました.txt"

print("------ Iterate all file ------")
local stream = io2.open(dir.."/"..file, "r")
for line in stream:lines() do
    print(line)
end
stream:close()

print("------ Read all file ------")
local stream = io2.open(dir.."/"..file, "r")
print(stream:readbytes(#stream + 1)) -- readbytes non null-terminated
stream:close()

print("------ Read all file 2 ------")
local stream = io2.open(dir.."/"..file, "r")
print(stream:read("*a")) -- readbytes non null-terminated
stream:close()

print("------ Rewrite/Create file ------")
local file2 = "Когда земляной червь танцует вальс и луна зеленеет, грядёт понимание смысла лопаты"
local stream = io2.open(dir.."/"..file2..".txt", "w")
stream:writeln(file2)
stream:write(42.24)
stream:write("\n")
stream:write(42)
stream:writebyte(0x0A) -- newline
stream:write("size=")
local size = stream:size()
stream:write(size + #tostring(size))
stream:close()

print("---------------------------------")
for file in io2.fs.dir() do
    print(io2.fs.permissions(file) .. " \"" .. file .. "\" isdir: " .. (io2.fs.isdir(file) and 'yes' or 'no'))
end
