
-- declare our protocol

isa_protocol = Proto("ISA","ISA Protocol")
local createSllTvb, dissectFPM, checkFpmLength
local isa_fields =
{
    RAW_msg = ProtoField.string("ISA.raw", "Raw Message", base.utf8),
    err_code = ProtoField.string("ISA.version", "Response", base.utf8),
    sender = ProtoField.string("ISA.sender", "Sender", base.utf8),
    client_command   = ProtoField.string ("ISA.command", "Command", base.utf8),
    payload   = ProtoField.string ("ISA.command", "Payload", base.utf8),
    payload_sub   = ProtoField.string ("ISA.command", "Message object", base.utf8),
    payload_body   = ProtoField.string ("ISA.command", "Message body", base.utf8),
    message_length   = ProtoField.uint8 ("ISA.length", "Length", base.DEC),
    request_id  = ProtoField.uint8 ("ISA.type", "Req_id", base.DEC, msgtype_valstr),
    opcode  = ProtoField.uint8 ("ISA.type", "Opcode", base.DEC, msgtype_valstr),
    response_to   = ProtoField.uint16("ISA.response", "Response", base.DEC),
    src_F = ProtoField.string("ISA.src","Source"),
    dst_F = ProtoField.string("ISA.dst","Destination"),
}
isa_protocol.fields = isa_fields
print("Fields registered\n")


local string_meta = getmetatable('')
function string_meta:__index( key )
    local val = string[ key ]
    if ( val ) then
        return val
    elseif ( tonumber( key ) ) then
        return self:sub( key, key )
    else
        error( "attempt to index a string value with bad key ('" .. tostring( key ) .. "' is not part of the string library)", 2 )
    end
end


 function Split(s, delimiter)
    result = {};
    for match in (s..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match);
    end
    return result;
end



 -- create a function to dissect it
 function isa_protocol.dissector(buffer,pinfo,tree)
    length = buffer:len()

    --Buffer nemusí obsahovat celou zprávu, protože tcp to segmentuje
    if buffer(length - 1, 1):string() ~= ")" then
        pktinfo.desegment_len = DESEGMENT_ONE_MORE_SEGMENT
        pktinfo.desegment_offset = 0
        return
    end

    if length == 0 then return end

    local tcp_src = tcp_src_f()
     local tcp_dst = tcp_dst_f()
     local ip_src = ip_src_f()
     local ip_dst = ip_dst_f()

    pinfo.cols.protocol:set("ISA Protocol")
    if string.find(tostring(pinfo.cols.info), "^ISA Protocol") == nil then
        pinfo.cols.info:set("ISA Protocol")
    end

 -- We start by adding our protocol to the dissection display tree.
    local tree_data = tree:add(isa_protocol, buffer:range(offset, length))

    --RAW message
   local raw_message = buffer:range(0, length)
   tree_data:add(isa_fields.RAW_msg, raw_message)

    --local opcode = buffer(4,4):le_int()
    -- local version = string.match(buffer, "ok")
    -- tree:add(isa_fields.err_code, version)
    string_raw = raw_message:string()
    
    parsed_args = Split(string_raw, " ")
    --print(#parsed_args)

    --client x server
    if((parsed_args[1] == "(ok") or (parsed_args[1] == "(err")) then
        sender_msg = "Server"
        err = (parsed_args[1]):sub(2)
        tree_data:add(isa_fields.err_code, err)
    else
        sender_msg = "Client"
    end
    tree_data:add(isa_fields.sender, sender_msg)

    

    --COMMAND
    command = (parsed_args[1]):sub(2)
    if(sender_msg == "Client") then
        tree_data:add(isa_fields.client_command, command)
    end

    --SOURCE and DESTINATION
    if tcp_src then
        local src = tostring(ip_src) .. ":" .. tostring(tcp_src)
        local dst = tostring(ip_dst) .. ":" .. tostring(tcp_dst)
        local conv = src  .. "->" .. dst
        tree_data:add(isa_fields.src_F,src)
        tree_data:add(isa_fields.dst_F,dst)
     end

    
    -- PROTOCOL
    if(sender_msg == "Client") then
        pinfo.cols.info:set("Request: ")
    else
        pinfo.cols.info:set("Response: ")
    end
    pinfo.cols.info:append(string_raw)



    if string.find(tostring(pinfo.cols.info), string_raw) == nil then
        if(sender_msg == "Client") then
            pinfo.cols.info:set("Request: ", string_raw)
            pinfo.cols.info:append(string_raw)

        else
            pinfo.cols.info:set("Response: ", string_raw)
            pinfo.cols.info:append(string_raw)

            
        end
    end

    --pojmenování protokolu
    pinfo.cols.protocol = isa_protocol.name
    pinfo.cols.protocol = "ISA Protocol"

    --dissector pro příkaz send
    if(command == "send") then
        --smazání uvozovek
        subject = string.gsub(parsed_args[4],'"'," ")
        body = string.gsub(parsed_args[5], '"', " ")
    end

    
    -- if(command == "list") then
    --     --smazání uvozovek
    --     print(parsed_args)
    --     --print(#parsed_args)
        
    -- end
    
        --dissector for list command
        i = 3
        no_data = (parsed_args[1] .. parsed_args[2] .. parsed_args[3])
        print(parsed_args[1])
        
        --packet data only for send,list and fetch
        if((no_data ~= "(ok\"registereduser") and (no_data ~= "(ok\"userlogged") and (parsed_args[1] ~= "(err")) then
            local subtree = tree:add(isa_protocol,buffer(),"ISA data:")
            while(parsed_args[i] ~= nil) do
                if(parsed_args[i+1] ~= nil) then
                length = parsed_args[i+1]:len() 

                subtree:add(isa_fields.payload_sub, parsed_args[i])
                --sub maže z těl zpráv ukončující závorky
                subtree:add(isa_fields.payload_body, parsed_args[i+1]:sub(1, -2))
                subtree:add(isa_fields.message_length, length)
                end
                subtree:add("\n")
                i = i+3
            end 
        end

   

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
 