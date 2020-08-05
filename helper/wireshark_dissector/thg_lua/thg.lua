shoal_protocol = Proto("SHOAL",  "shoal Protocol")

gal_size = ProtoField.uint16("shoal.gal_size", "size", base.DEC)
gal_id = ProtoField.uint8("shoal.gal_id", "id", base.DEC)
gal_dest = ProtoField.uint8("shoal.gal_dest", "dest", base.DEC)

type = ProtoField.uint8("shoal.type", "type", base.HEX)
src = ProtoField.uint16("shoal.src", "src", base.DEC)
dst = ProtoField.uint16("shoal.dst", "dst", base.DEC)
payload_size = ProtoField.uint16("shoal.payload_size", "payload_size", base.DEC)
handler = ProtoField.uint8("shoal.handler", "handler", base.DEC, nil, 0x0F)
handler_args = ProtoField.uint8("shoal.handler_args", "handler_args", base.DEC, nil, 0xF0)
payload = ProtoField.none("shoal.payload", "payload")

shoal_protocol.fields = {
    gal_size, gal_id, gal_dest, -- galapagos header
    type, src, dst, payload_size, handler, handler_args, -- shoal header
    payload
}

function shoal_protocol.dissector(buffer, pinfo, tree)
    length = buffer:len()
    if length == 0 then return end

    pinfo.cols.protocol = shoal_protocol.name

    local subtree = tree:add(shoal_protocol, buffer(), "shoal")
    local gal_subtree = subtree:add(shoal_protocol, buffer(), "galapagos")
    local header_subtree = subtree:add(shoal_protocol, buffer(), "header")

    -- parse galapagos header
    gal_subtree:add_le(gal_size, buffer(0,2)) -- read two bytes from offset 0
    gal_subtree:add_le(gal_id, buffer(2,1))
    gal_subtree:add_le(gal_dest, buffer(3,1))

    -- parse shoal header
    header_subtree:add_le(type, buffer(8,1))
    header_subtree:add_le(src, buffer(9,2))
    header_subtree:add_le(dst, buffer(11,2))
    header_subtree:add_le(payload_size, buffer(13,2))
    header_subtree:add_le(handler, buffer(15,1))
    header_subtree:add_le(handler_args, buffer(15,1))

    subtree:add_le(payload, buffer(16, length-16))
end

local tcp_port = DissectorTable.get("tcp.port")
tcp_port:add(32768, shoal_protocol)
