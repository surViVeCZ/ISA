
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


function isa_protocol.dissector(tvbuf, pktinfo, root)
    print("fpm_proto.dissector called")
    -- reset the save Tvbs
    tvbs = {}

    -- get the length of the packet buffer (Tvb).
    local pktlen = tvbuf:len()
    local bytes_consumed = 0

    -- we do this in a while loop, because there could be multiple FPM messages
    while bytes_consumed < pktlen do

        local result = dissectFPM(tvbuf, pktinfo, root, bytes_consumed)

        if result > 0 then
            -- we successfully processed an FPM message, of 'result' length
            bytes_consumed = bytes_consumed + result
            -- go again on another while loop
        elseif result == 0 then
            -- If the result is 0, then it means we hit an error of some kind,
            return 0
        else
            -- we need more bytes, so set the desegment_offset to what we
            -- already consumed, and the desegment_len to how many more
            -- are needed
            pktinfo.desegment_offset = bytes_consumed

            -- invert the negative result so it's a positive number
            result = -result
            pktinfo.desegment_len = result

            -- even though we need more bytes, this packet is for us, so we
            -- tell wireshark all of its bytes are for us by returning the
            -- number of Tvb bytes we "successfully processed"
            return pktlen
        end        
    end
    return bytes_consumed
end


 -- create a function to dissect it
 function isa_protocol.dissector(buffer,pinfo,tree)
    length = buffer:len()

    if length == 0 then return end

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
    

  
    command = (parsed_args[1]):sub(2)
    if(sender_msg == "Client") then
        tree_data:add(isa_fields.client_command, command)
    end

    
    -- lišta protocol info
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


    -- if((command == "send") or (command == "fetch") or (command == "list")) then
    --     local subtree = tree:add(isa_protocol,buffer(),"ISA data:")
    --     subtree:add(isa_fields.message_length, length)
    --     subtree:add(isa_fields.payload_sub, subject)
    --     subtree:add(isa_fields.payload_body, body)
    -- end
    
        --dissector for list command
        i = 3
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
