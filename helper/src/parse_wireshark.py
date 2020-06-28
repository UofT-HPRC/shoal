import argparse
import json
import os
import urllib.request
import copy
import math

import ijson

BYTES_PER_WORD = 8

def get_word(data, index):
    word_bytes = [data[i:i+2] for i in range(index+0, index+(BYTES_PER_WORD*2), 2)]
    word = ""
    for j in range(BYTES_PER_WORD):
        word = word_bytes[j] + word
    return word

def get_packet(data):
    data_stream = data.replace(":", "")
    word_count = int(math.ceil(len(data_stream)/BYTES_PER_WORD))
    words = []
    for i in range(word_count):
        words.append(get_word(data_stream, i*BYTES_PER_WORD*2))
    return words

def save_adjacent_packets(file_name, data_stream, prev_packet):
    i = prev_packet[0]
    end_point = min(prev_packet[1] + 1124, len(data_stream)) 
    with open("wireshark_data.txt", "w") as f:
        while(i < end_point):
            payload = get_word(data_stream, i)
            i += (2*BYTES_PER_WORD)
            f.write(payload)
            f.write("\n")

        f.write("\n")

        url_name = "file://" + os.path.abspath(file_name)
        f_2 = urllib.request.urlopen(url_name)
        packets = ijson.items(f_2, "item")

        data_stream_len = 0
        found = False
        found_2 = False
        for packet in packets:
                echo_data = str(packet["_source"]["layers"]["echo"]["echo.data"])
                if found_2:
                    packet_data = get_packet(echo_data)
                    f.write("\n".join(packet_data))
                    f.write("\n")
                    break
                if found:
                    packet_data = get_packet(echo_data)
                    f.write("\n".join(packet_data))
                    f.write("\n")
                    found_2 = True
                if data_stream_len <= prev_packet[0] and data_stream_len + len(echo_data.replace(":", "")) >= prev_packet[0]:
                    frame_number = str(packet["_source"]["layers"]["frame"]["frame.number"])
                    f.write(f"Frame number: {frame_number}\n")
                    packet_data = get_packet(echo_data)
                    f.write("\n".join(packet_data))
                    f.write("\n")
                    found = True
                data_stream_len += len(echo_data.replace(":", ""))            

def save_packets(file_name, packet_count):
    url_name = "file://" + os.path.abspath(file_name)
    f = urllib.request.urlopen(url_name)
    packets = ijson.items(f, "item")

    data_stream = ""
    i = 0
    with open("wireshark_data.txt", "w") as f:
        for packet in packets:
            if i == packet_count - 1:
                echo_data = str(packet["_source"]["layers"]["echo"]["echo.data"])
                packet_data = get_packet(echo_data)
                f.write("\n".join(packet_data))
                f.write("\n")
            elif i == packet_count:
                echo_data = str(packet["_source"]["layers"]["echo"]["echo.data"])
                packet_data = get_packet(echo_data)
                f.write("\n".join(packet_data))
                f.write("\n")
            elif i == packet_count + 1:
                echo_data = str(packet["_source"]["layers"]["echo"]["echo.data"])
                packet_data = get_packet(echo_data)
                f.write("\n".join(packet_data))
                f.write("\n")
                break
            i += 1

def parse(file_name):
    url_name = "file://" + os.path.abspath(file_name)
    f = urllib.request.urlopen(url_name)
    packets = ijson.items(f, "item")

    data_stream = ""
    for packet in packets:
        echo_data = str(packet["_source"]["layers"]["echo"]["echo.data"])
        tcp = packet["_source"]["layers"]["tcp"]
        if "tcp.analysis" in tcp:
            tcp_analysis = tcp["tcp.analysis"]
            retransmission = False
            if "tcp.analysis.flags" in tcp_analysis:
                for key, value in tcp_analysis["tcp.analysis.flags"].items():
                    # print(value)
                    # exit(0)
                    if "retransmission" in value["_ws.expert.message"]:
                        retransmission = True
                        break
        if not retransmission:
            data_stream += echo_data.replace(":", "")

    prev_packet = [0, 0]
    i = 0
    str_len = len(data_stream)
    packet_count = 0
    while(i < str_len):
        header = get_word(data_stream, i)
        prev_packet[0] = copy.deepcopy(prev_packet[1])
        prev_packet[1] = copy.deepcopy(i)
        i += (2*BYTES_PER_WORD)
        
        packet_size = int(header[-4:])
        if packet_size > 1124 or packet_size < 2:
            print(f"Error at index {i}, length of stream is {str_len}")
            print(prev_packet)
            print(header)
            print(packet_count)
            save_adjacent_packets(file_name, data_stream, prev_packet)
            # save_packets(file_name, packet_count)
            break

        payloads = []
        for j in range(packet_size):
            payload = get_word(data_stream, i)
            i += (2*BYTES_PER_WORD)
            payloads.append(payload)

        packet_count += 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Parse the JSON output from wireshark")
    parser.add_argument("file_name", type=str, help="JSON file to parse")
    
    args = parser.parse_args()

    file_name = args.file_name

    if os.path.exists(file_name):
        parse(file_name)
    else:
        print(f"File {file_name} not found")
