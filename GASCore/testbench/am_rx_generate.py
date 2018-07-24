def main():

    testFileName = "testbench/am_rx.dat"
    f_test = open(testFileName, "w+")

    #Long Message C
    f_test.write("Token 0xAAAA 0 0\n") #token: AAAA
    f_test.write("axis_net 0x0114CCAA 0 1\n") #header: 0x113 words coming from AA to CC
    f_test.write("axis_net 0x00FF0006 0 1\n") #AM header: Long vector message with 0 args calling handler FF
    f_test.write("axis_net 0x00010C20 0 1\n") #Vector payload and number
    f_test.write("axis_net 0x0000000C 0 0\n") #Payload size: 12 words
    f_test.write("axis_net 0x00000000 0 0\n") #destination address (lower)
    f_test.write("axis_net 0x00000000 0 0\n") #destination address (upper)
    f_test.write("axis_net 0x00000100 0 0\n") #Payload size: 256 words
    f_test.write("axis_net 0x0000FF00 0 0\n") #destination address (lower)
    f_test.write("axis_net 0x00000000 0 1\n") #destination address (upper)
    for count in range(2):
        for payload in range(3):
            f_test.write("axis_net 0xBEEFDEAD 0 0\n") #Payload 0-2, 4-6
        f_test.write("axis_net 0xDEADBEEF 0 0\n") #Payload 3, 7
    for payload in range(3):
        f_test.write("axis_net 0xBEEFDEAD 0 0\n") #Payload 8-10
    f_test.write("axis_net 0xDEADBEEF 0 0\n") #Payload 11
    for payload in range(255):
        f_test.write("axis_net 0xFFAAFFAA 0 0\n") #Payload 0-254
    f_test.write("axis_net 0xDDDDDDDD 1 1\n") #Payload 255
    f_test.write("axis_s2mmStatus 0x80 0 0\n") #status good
    f_test.write("END 0x00000006 0 0\n")

    f_test.write("axis_handler 0x00FF0006 0 0\n") #AM header
    f_test.write("axis_handler 0x0000AAAA 0 0\n") #token
    f_test.write("axis_handler 0x00010C20 0 0\n") #Vector payload and number
    f_test.write("axis_handler 0x0000000C 0 0\n") #Payload size
    f_test.write("axis_handler 0x00000000 0 0\n") #destination address (lower)
    f_test.write("axis_handler 0x00000000 0 0\n") #destination address (upper)
    f_test.write("axis_handler 0x00000100 0 0\n") #Payload size: 256 words
    f_test.write("axis_handler 0x0000FF00 0 0\n") #destination address (lower)
    f_test.write("axis_handler 0x00000000 0 0\n") #destination address (upper)
    f_test.write("axis_s2mmCommand 0x000000000080000C 0 0\n")
    for arg in range(3):
        f_test.write("axis_s2mm 0xBEEFDEAD 0 0\n") #payload
    f_test.write("axis_s2mm 0xDEADBEEF 0 0\n") #payload
    for arg in range(3):
        f_test.write("axis_s2mm 0xBEEFDEAD 0 0\n") #payload
    f_test.write("axis_s2mm 0xDEADBEEF 0 0\n") #payload
    for arg in range(3):
        f_test.write("axis_s2mm 0xBEEFDEAD 0 0\n") #payload
    f_test.write("axis_s2mm 0xDEADBEEF 1 0\n") #payload
    f_test.write("axis_s2mmCommand 0x0000FF0040800100 0 0\n")
    for arg in range(255):
        f_test.write("axis_s2mm 0xFFAAFFAA 0 0\n") #payload
    f_test.write("axis_s2mm 0xDDDDDDDD 1 0\n") #payload
    f_test.write("END 0x00000006 1 0\n")

    #Long Message B
    f_test.write("Token 0xAAAA 0 0\n") #token: AAAA
    f_test.write("axis_net 0x0015CCAA 0 1\n") #header: 0x19 words coming from AA to CC
    f_test.write("axis_net 0x00FF0205 0 1\n") #AM header: Long strided message with 2 args calling handler FF
    f_test.write("axis_net 0x0000000C 0 1\n") #Payload size: 12 words
    f_test.write("axis_net 0x00000000 0 0\n") #destination address (lower)
    f_test.write("axis_net 0x00000000 0 1\n") #destination address (upper)
    f_test.write("axis_net 0x00000004 0 0\n") #contiguous block size
    f_test.write("axis_net 0x00000004 0 0\n") #number of cont. blocks
    f_test.write("axis_net 0x00000100 0 1\n") #stride between blocks
    f_test.write("axis_net 0xAABBCCDD 0 0\n") #handler arg 0
    f_test.write("axis_net 0xDDCCBBAA 0 1\n") #handler arg 1
    for count in range(2):
        for payload in range(3):
            f_test.write("axis_net 0xBEEFDEAD 0 0\n") #Payload 0-2, 4-6
        f_test.write("axis_net 0xDEADBEEF 0 0\n") #Payload 3, 7
    for payload in range(3):
        f_test.write("axis_net 0xBEEFDEAD 0 0\n") #Payload 8-10
    f_test.write("axis_net 0xDEADBEEF 1 1\n") #Payload 11
    f_test.write("axis_s2mmStatus 0x80 0 0\n") #status good
    f_test.write("END 0x00000005 0 0\n")

    f_test.write("axis_handler 0x00FF0205 0 0\n") #AM header
    f_test.write("axis_handler 0x0000AAAA 0 0\n") #token
    f_test.write("axis_handler 0x0000000C 0 0\n") #Payload size
    f_test.write("axis_handler 0x00000000 0 0\n") #destination address (lower)
    f_test.write("axis_handler 0x00000000 0 0\n") #destination address (upper)
    f_test.write("axis_handler 0x00000004 0 0\n") #contiguous block size
    f_test.write("axis_handler 0x00000004 0 0\n") #number of cont. blocks
    f_test.write("axis_handler 0x00000100 0 0\n") #stride between blocks
    f_test.write("axis_handler 0xAABBCCDD 0 0\n") #handler arg 0
    f_test.write("axis_handler 0xDDCCBBAA 0 0\n") #handler arg 1
    f_test.write("axis_s2mmCommand 0x0000000040800004 0 0\n")
    for arg in range(3):
        f_test.write("axis_s2mm 0xBEEFDEAD 0 0\n") #payload
    f_test.write("axis_s2mm 0xDEADBEEF 1 0\n") #payload
    f_test.write("axis_s2mmCommand 0x0000010040800004 0 0\n")
    for arg in range(3):
        f_test.write("axis_s2mm 0xBEEFDEAD 0 0\n") #payload
    f_test.write("axis_s2mm 0xDEADBEEF 1 0\n") #payload
    f_test.write("axis_s2mmCommand 0x0000020040800004 0 0\n")
    for arg in range(3):
        f_test.write("axis_s2mm 0xBEEFDEAD 0 0\n") #payload
    f_test.write("axis_s2mm 0xDEADBEEF 1 0\n") #payload
    f_test.write("END 0x00000005 1 0\n")

    #Long Message A
    f_test.write("Token 0xAAAA 0 0\n") #token: AAAA
    f_test.write("axis_net 0x0104CCAA 0 1\n") #header: 0x104 words coming from AA to CC
    f_test.write("axis_net 0x00FF0004 0 1\n") #AM header: Long message with 0 args calling handler FF
    f_test.write("axis_net 0x00000100 0 1\n") #Payload size: 256 words
    f_test.write("axis_net 0xAABBCCD8 0 0\n") #destination address (lower)
    f_test.write("axis_net 0x00000000 0 1\n") #destination address (upper)
    for payload in range(255):
        f_test.write("axis_net 0xDEADBEEF 0 0\n") #Payload 0-254
    f_test.write("axis_net 0xDEADBEEF 1 1\n") #Payload 255
    f_test.write("axis_s2mmStatus 0x80 0 0\n") #status good
    f_test.write("END 0x00000004 0 0\n")

    f_test.write("axis_handler 0x00FF0004 0 0\n") #AM header
    f_test.write("axis_handler 0x0000AAAA 0 0\n") #token
    f_test.write("axis_handler 0x00000100 0 0\n") #Payload size
    f_test.write("axis_handler 0xAABBCCD8 0 0\n") #destination address (lower)
    f_test.write("axis_handler 0x00000000 0 0\n") #destination address (upper)
    f_test.write("axis_s2mmCommand 0xAABBCCD840800100 0 0\n")
    for arg in range(255):
        f_test.write("axis_s2mm 0xDEADBEEF 0 0\n") #payload
    f_test.write("axis_s2mm 0xDEADBEEF 1 0\n") #payload
    f_test.write("END 0x00000004 1 0\n")

    #Medium Message B
    f_test.write("Token 0xAAAA 0 0\n") #token: AAAA
    f_test.write("axis_net 0x0108CCAA 0 1\n") #header: 108 words coming from AA to CC
    f_test.write("axis_net 0x00FF0602 0 1\n") #AM header: Medium message with 6 args calling handler FF
    f_test.write("axis_net 0x00000100 0 1\n") #Payload size: 256 words
    for arg in range(5):
        f_test.write("axis_net 0x98765432 0 0\n") #handler arg 0-4
    f_test.write("axis_net 0x98765432 0 1\n") #handler arg 5
    for payload in range(255):
        f_test.write("axis_net 0xDEADBEEF 0 0\n") #Payload 0-254
    f_test.write("axis_net 0xDEADBEEF 1 1\n") #Payload 255
    f_test.write("END 0x00000003 0 0\n")

    f_test.write("axis_handler 0x00FF0602 0 0\n") #AM header
    f_test.write("axis_handler 0x0000AAAA 0 0\n") #token
    f_test.write("axis_handler 0x00000100 0 0\n") #Payload size
    for arg in range(6):
        f_test.write("axis_handler 0x98765432 0 0\n") #handler args
    for arg in range(255):
        f_test.write("axis_handler 0xDEADBEEF 0 0\n") #payload
    f_test.write("axis_handler 0xDEADBEEF 1 0\n") #payload
    f_test.write("END 0x00000003 1 0\n")

    #Medium Message A
    f_test.write("Token 0xAAAA 0 0\n") #token: AAAA
    f_test.write("axis_net 0x0003CCAA 0 1\n") #header: 3 words coming from AA to CC
    f_test.write("axis_net 0x00FF0002 0 1\n") #AM header: Medium message with 0 args calling handler FF
    f_test.write("axis_net 0x00000001 0 1\n") #Payload size: 1 word
    f_test.write("axis_net 0xDEADBEEF 1 1\n") #Payload: DEADBEEF
    f_test.write("END 0x00000002 0 0\n")

    f_test.write("axis_handler 0x00FF0002 0 0\n") #AM header
    f_test.write("axis_handler 0x0000AAAA 0 0\n") #token
    f_test.write("axis_handler 0x00000001 0 0\n") #Payload size
    f_test.write("axis_handler 0xDEADBEEF 1 0\n") #payload
    f_test.write("END 0x00000002 1 0\n")

    #Short Message B
    f_test.write("Token 0xDCBA 0 0\n") #token
    f_test.write("axis_net 0x0001CCAA 0 1\n") #header
    f_test.write("axis_net 0x00DD0001 1 1\n") #AM header
    f_test.write("END 0x00000001 0 0\n")

    f_test.write("axis_handler 0x00DD0001 0 0\n") #AM header
    f_test.write("axis_handler 0x0000DCBA 1 0\n") #Token
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
    f_test.write("END 0x00000000 1 1\n")

    f_test.close()

if __name__ == "__main__":
    main()