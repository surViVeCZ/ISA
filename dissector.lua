
-- declare our protocol
 isa_protocol = Proto("ISA","ISA Protocol")

--  message_length = ProtoField.int32("ISA.message_length", "messageLength", base.DEC)
--  request_id     = ProtoField.int32("ISA.requestid"     , "requestID"    , base.DEC)
--  response_to    = ProtoField.int32("ISA.responseto"    , "responseTo"   , base.DEC)
--  opcode         = ProtoField.int32("ISA.opcode"        , "opCode"       , base.DEC)

 isa_protocol.fields = { message_length, request_id, response_to, opcode }

 function Split(s, delimiter)
    result = {};
    for match in (s..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match);
    end
    return result;
end


-- function isempty(s)
--     return s == nil or s == ''
--   end


 -- create a function to dissect it
 function isa_protocol.dissector(buffer,pinfo,tree)
    length = buffer:len()
    if length == 0 then return end

    local data = tostring(buffer())

   
    for i = 3, 1024, 3 do --start,do kdy, inkrementace
        data = data:gsub('()',{[i]=':'})
    end
     
    print(data)

    
    s = ""
    for i in string.gmatch(data, "[^:]*") do
        if i ~= nil and i ~= '' then
            --print(string.char(tonumber(i,16)))
            s = s .. string.char(tonumber(i,16))
        end
    end
    print( s )
    -- arguments = Split(data, "\n")
    -- print(arguments[1])
    -- print(arguments[2])

    --local opcode = buffer(4,4):le_int()
    --local version = string.match(buffer, "ok")
    

    pinfo.cols.protocol = isa_protocol.name
    pinfo.cols.protocol = "ISA Protocol"
    local subtree = tree:add(isa_protocol,buffer(),"ISA data:")

    subtree:add(buffer(0,2),"Data length: ", length)
    subtree:add(buffer(2,2),"Sender: ", version)
    subtree:add(buffer(4,2),"Type: ".. buffer(4,2):uint())
    subtree:add(buffer(6,2),"Command: ", opcode)
    subtree = subtree:add(buffer(8,2),"Payload: ")
    subtree:add(buffer(8,1),"Payload length: " .. buffer(8,1):uint())
    subtree:add(buffer(9,1),"Payload raw: ".. buffer(9,1):uint())
 end


 tcp_table = DissectorTable.get("tcp.port")
 -- register our protocol to handle tcp port 32323
 tcp_table:add(32323,isa_protocol)



 --------------------------------------------------------------------
 -- trivial postdissector example
 -- declare some Fields to be read
 ip_src_f = Field.new("ip.src")
 ip_dst_f = Field.new("ip.dst")
 tcp_src_f = Field.new("tcp.srcport")
 tcp_dst_f = Field.new("tcp.dstport")
 -- declare our (pseudo) protocol
 isa_protocol = Proto("trivial","ISA protocol")
 -- create the fields for our "protocol"
 src_F = ProtoField.string("trivial.src","Source")
 dst_F = ProtoField.string("trivial.dst","Destination")
 conv_F = ProtoField.string("trivial.conv","Conversation","A Conversation")
 -- add the field to the protocol
 isa_protocol.fields = {src_F, dst_F, conv_F}
 -- create a function to "postdissect" each frame
 function isa_protocol.dissector(buffer,pinfo,tree)
     -- obtain the current values the protocol fields
     local tcp_src = tcp_src_f()
     local tcp_dst = tcp_dst_f()
     local ip_src = ip_src_f()
     local ip_dst = ip_dst_f()
     if tcp_src then
        local subtree = tree:add(isa_protocol,"ISA Protocol Data")
        local src = tostring(ip_src) .. ":" .. tostring(tcp_src)
        local dst = tostring(ip_dst) .. ":" .. tostring(tcp_dst)
        local conv = src  .. "->" .. dst
        subtree:add(src_F,src)
        subtree:add(dst_F,dst)
        subtree:add(conv_F,conv)
     end
 end

 -- register our protocol as a postdissector
 --register_postdissector(isa_protocol)
