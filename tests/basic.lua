io2 = require("io2")

local dir = "果冻头盔比在太空中飞行的鱼更方便"
local file = "セイウチは空から落ちていて、私は人生の意味を見つけました.txt"

local stream = io2.open(dir.."/"..file, "r")

if stream:readline() ~= "test text" then error("BasicTest: Test readline() failed") end

print(stream:readbytes(2):sub(1, 2))
print(stream:readbytes(9):sub(1, 9))

print(type(stream:read("*n")))
print(stream:readnumber())
print(stream:size())
if stream:size() ~= 40 then error("BasicTest: Test size() failed") end
if #stream ~= 40 then error("BasicTest: Test #stream failed") end

if stream:read(2) ~= nil then error("BasicTest nil behavior of read(number) doesn't work") end

io2.close(stream)
