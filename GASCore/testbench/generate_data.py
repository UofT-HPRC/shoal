testFileName = "am_rx.dat"

def main():

    f_test = open(testFileName, "w+")

    #Medium Message B
    f_test.write("Token 0xAAAA 0 0\n") #token: AAAA
    f_test.write("axis_net 0x0003CCAA 0 1\n") #header: 3 words coming from AA to CC
    f_test.write("axis_net 0x00FF0602 0 1\n") #AM header: Medium message with 6 args calling handler FF
    f_test.write("axis_net 0x000000FF 0 1\n") #Payload size: 256 words
    for arg in range(5):
        f_test.write("axis_net 0x98765432 0 0\n") #handler arg 0-5
    f_test.write("axis_net 0x98765432 0 1\n") #handler arg 5
    for payload in range(255):
        f_test.write("axis_net 0xDEADBEEF 0 0\n") #Payload 0-254
    f_test.write("axis_net 0xDEADBEEF 1 1\n") #Payload 255
    f_test.write("END 0x00000003 0 0\n")

    f_test.write("axis_handler 0x00FF0002 0 0\n") #AM header
    f_test.write("axis_handler 0x0000AAAA 0 0\n") #token
    f_test.write("axis_handler 0x000000FF 0 0\n") #Payload size
    for arg in range(5):
        f_test.write("axis_handler 0x98765432 0 0\n") #handler args
    for arg in range(256):
        f_test.write("axis_handler 0xDEADBEEF 0 0\n") #payload
    f_test.write("END 0x00000003 1 1\n")

    #Medium Message A
    f_test.write("Token 0xAAAA 0 0\n") #token: AAAA
    f_test.write("axis_net 0x0003CCAA 0 1\n") #header: 3 words coming from AA to CC
    f_test.write("axis_net 0x00FF0002 0 1\n") #AM header: Medium message with 0 args calling handler FF
    f_test.write("axis_net 0x00000001 0 1\n") #Payload size: 1 word
    f_test.write("axis_net 0xDEADBEEF 0 1\n") #Payload: DEADBEEF
    f_test.write("END 0x00000002 0 0\n")

    f_test.write("axis_handler 0x00FF0002 0 0\n") #AM header
    f_test.write("axis_handler 0x0000AAAA 0 0\n") #token
    f_test.write("axis_handler 0x00000001 0 0\n") #Payload size
    f_test.write("axis_handler 0xDEADBEEF 0 0\n") #payload
    f_test.write("END 0x00000002 1 0\n")

    #Short Message B
    f_test.write("Token 0xDCBA 0 0\n") #token
    f_test.write("axis_net 0x0001CCAA 0 1\n") #header
    f_test.write("axis_net 0x00DD0001 1 1\n") #AM header
    f_test.write("END 0x00000001 0 0\n")

    f_test.write("axis_handler 0x00DD0001 0 0\n") #AM header
    f_test.write("axis_handler 0x0000DCBA 0 0\n") #Token
    f_test.write("END 0x00000001 1 0\n")

    #Short Message A
    f_test.write("Token 0xABCD 0 0\n") #token
    f_test.write("axis_net 0x0003CCAA 0 1\n") #header
    f_test.write("axis_net 0x00DD0201 0 1\n") #AM header
    f_test.write("axis_net 0xDEADBEEF 0 0\n") #handler arg 0
    f_test.write("axis_net 0xFEEDDEED 1 1\n") #handler arg 1
    f_test.write("END 0x00000000 0 0\n")

    f_test.write("axis_handler 0x00DD0201 0 0\n") #AM header
    f_test.write("axis_handler 0x0000ABCD 0 0\n") #Token
    f_test.write("axis_handler 0xDEADBEEF 0 0\n") #handler arg 0
    f_test.write("axis_handler 0xFEEDDEED 1 0\n") #handler arg 1
    f_test.write("END 0x00000000 1 0\n")

    f_test.close()

if __name__ == "__main__":
    main()