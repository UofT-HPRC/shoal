import argparse
import math

WORD_BITS = 64
WORD_BYTES = int(WORD_BITS/8)

def parse_galapagos_header(words):
    header = words[0]
    word_count = header[-4:]
    packet_id = header[-6:-4]
    dest = header[-8:-6]
    print("Galapagos Header")
    print("    TUSER (word count): " + word_count)
    print("    TID.. (TCP ID)....: " + packet_id)
    print("    TDEST.............: " + dest)
    return words[1:]

def parse_shoal(words):
    header = words[0]
    am_type = header[-2:]
    am_src = header[-6:-2]
    am_dst = header[-10:-6]
    am_size = header[-14:-10]
    am_handler = header[-15:-14]
    am_args = header[-16:-15]
    print("Shoal Header")
    print("    Type: " + am_type)
    print("    Src: " + am_src)
    print("    Dst: " + am_dst)
    print("    Size: " + am_size)
    print("    Handler: " + am_handler)
    print("    Args: " + am_args)

    print("Remaining words")
    for word in words[1:]:
        print("    " + word)

def get_words(packet_bytes):
    word_count = int(math.ceil(len(packet_bytes)/WORD_BYTES))
    words = []
    for i in range(word_count):
        word = ""
        for j in range(WORD_BYTES):
            word = packet_bytes[i*WORD_BYTES + j] + word
        words.append(word)
    return words

def decode(packet):
    packet_bytes = [packet[i:i+2] for i in range(0, len(packet), 2)]
    words = get_words(packet_bytes)
    
    words = parse_galapagos_header(words)
    print("")
    parse_shoal(words)

if __name__ == "__main__":
    # parser = argparse.ArgumentParser()
    # parser.add_argument("packet", type=str, help="packet to decode")
    # args = parser.parse_args()

    # decode(args.packet)

    # from master to slave: gratituous arp request from 10.1.2.101 to 10.1.2.101
    packet = "16faffffffffffff0100060802ca553e16fa0100040600086502010a02ca553e010affffffffffff000000000000650200000000000000000000000000000000"

    # from master to slave: TCP syn (sent after ~1us)
    # this was initiated after the GAScore on master tried sending something
    packet = "16fa02ca553e16fa0045000802ca553e0640000000002c00010a6502010a636223560700008003020a6000000000b00104020000daa7ffff000000000000b405"

    # from slave to master: TCP syn/ack (sent after ~1us)
    packet = "16fa02ca553e16fa0045000802ca553e0640000000002c00010a0302010a636223560080070065021a60b1012356b00104020000f64fffff000000000000b405"

    # from master to slave: TCP ack (sent after ~0.8us)
    packet = "16fa02ca553e16fa0045000802ca553e0640000000002800010a6502010a676223560700008003021850b1012356b10100000000b367ffff0000000000000000"
    
    # from master to slave: TCP data (100001010000000001000001000000030000000000000000)
    # Galapagos Header
    #     TUSER (word count): 0010
    #     TID.. (TCP ID)....: 01
    #     TDEST.............: 01

    # Shoal Header
    #     Type: 01
    #     Src: 0000
    #     Dst: 0001
    #     Size: 0000
    #     Handler: 3
    #     Args: 0
    # Remaining words
    #     0000000000000000
    packet = "16fa02ca553e16fa0045000802ca553e0640000000004000010a6502010a4f6223560700008003021850b1012356b101001000009655ffff000100000000010100000300000001000000000000000000"

    # from slave to master: TCP syn
    packet = "16fa02ca553e16fa0045000802ca553e0640000000002c00010a0302010a63623be70080070065020a6000000000310c04020000410cffff000000000000b405"

    # two more TCP transactions, one from master to slave (probably syn/ack), and then from slave to master (probably ack)
    # this was initiated after the GAScore on slave tried sending something
    
    
    # packet_bytes = [packet[i:i+2] for i in range(0, len(packet), 2)]
    # words = get_words(packet_bytes)
    # print("".join(words))

    # decode("100001010000000001000001000000030000000000000000")

    words = ["0000080001000012"]
    parse_shoal(words)