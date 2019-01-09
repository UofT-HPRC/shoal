from sonar.core.include.utilities import extractNumber

def strToInt(packet):
    packetArgs = packet[1:-1].split(",")
    intVal = 0

    if packetArgs[0] == "AMHeader":
        assert (len(packetArgs) == 7),"Invalid number of arguments for AMHeader"
        if ":" not in packetArgs[1]:
            argSrc = packetArgs[1]
            argDst = packetArgs[2]
            argPayload = packetArgs[3]
            argHandler = packetArgs[4]
            argPacketType = packetArgs[5]
            argArguments = packetArgs[6]
        else:
            for arg in packetArgs[1:]:
                argList = arg.split(":")
                if argList[0] == "src":
                    argSrc = argList[1]
                elif argList[0] == "dst":
                    argDst = argList[1]
                elif argList[0] == "payload":
                    argPayload = argList[1]
                elif argList[0] == "handler":
                    argHandler = argList[1]
                elif argList[0] == "type":
                    argPacketType = argList[1]
                elif argList[0] == "args":
                    argArguments = argList[1]
                else:
                    printError(1, "Invalid key for AMHeader")
                    exit(1)
        
        src = extractNumber(argSrc) << 8
        dst = extractNumber(argDst) << 24
        payload = extractNumber(argPayload) << 40
        handler = extractNumber(argHandler) << 52
        packetType = extractNumber(argPacketType)
        arguments = extractNumber(argArguments) << 56

        intVal = src + dst + payload + handler + packetType + arguments

    elif packetArgs[0] == "AMLongVector":
        assert (len(packetArgs) == 4 or len(packetArgs) == 6),\
            "Invalid number of arguments for AMLongVector"
        
        if ":" not in packetArgs[1]:
            if len(packetArgs) == 4:
                argDst = packetArgs[1]
                argDstSize = packetArgs[2]
                argToken = packetArgs[3]
            else:
                argSrc = packetArgs[1]
                argDst = packetArgs[2]
                argSrcSize = packetArgs[3]
                argDstSize = packetArgs[4]
                argToken = packetArgs[5]
        else:
            for arg in packetArgs[1:]:
                argList = arg.split(":")
                if argList[0] == "src":
                    argDst = argList[1]
                if argList[0] == "dst":
                    argDst = argList[1]
                elif argList[0] == "srcSize":
                    argSrcSize = argList[1]
                elif argList[0] == "dstSize":
                    argDstSize = argList[1]
                elif argList[0] == "token":
                    argToken = argList[1]
                else:
                    printError(1, "Invalid key for AMLongVector")
                    exit(1)

        if len(packetArgs) == 4:
            dst = extractNumber(argDst) << 4
            dstSize = extractNumber(argDstSize) << 20
            token = extractNumber(argToken) << 40
            intVal = dst + dstSize + token
        else:
            src = extractNumber(argSrc)
            dst = extractNumber(argDst) << 4
            srcSize = extractNumber(argSrcSize) << 8
            dstSize = extractNumber(argDstSize) << 20
            token = extractNumber(argToken) << 40
            intVal = src + dst + srcSize + dstSize + token

    elif packetArgs[0] == "AMLongStride":
        assert (len(packetArgs) == 4 or len(packetArgs) == 5), \
            "Invalid number of arguments for AMLongStride"

        if ":" not in packetArgs[1]:
            argStride = packetArgs[1]
            argBlockSize = packetArgs[2]
            argBlockNum = packetArgs[3]
            if len(packetArgs) == 5:
                argToken = packetArgs[4]
        else:
            for arg in packetArgs[1:]:
                argList = arg.split(":")
                if argList[0] == "stride":
                    argStride = argList[1]
                if argList[0] == "blockSize":
                    argBlockSize = argList[1]
                elif argList[0] == "blockNum":
                    argBlockNum = argList[1]
                elif argList[0] == "token":
                    argToken = argList[1]
                else:
                    printError(1, "Invalid key for AMLongStride")
                    exit(1)
        
        stride = extractNumber(argStride)
        blockSize = extractNumber(argBlockSize) << 16
        blockNum = extractNumber(argBlockNum) << 28
        if(len(packetArgs) == 5):
            token = extractNumber(argToken) << 40
            intVal += token
        intVal += stride + blockNum + blockSize

    elif packetArgs[0] == "dataMoverCommand":
        assert (len(packetArgs) == 8 or len(packetArgs) == 10),\
            "Invalid number of arguments for dataMoverCommand"

        if ":" not in packetArgs[1]:
            argBTT = packetArgs[1]
            argCommandType = packetArgs[2]
            argDSA = packetArgs[3]
            argEOF = packetArgs[4]
            argDRR = packetArgs[5]
            argAddr = packetArgs[6]
            argTag = packetArgs[7]
            if len(packetArgs) == 10:
                argUser = packetArgs[8]
                argCache = packetArgs[9]
        else:
            for arg in packetArgs[1:]:
                argList = arg.split(":")
                if argList[0] == "btt":
                    argBTT = argList[1]
                if argList[0] == "type":
                    argCommandType = argList[1]
                elif argList[0] == "dsa":
                    argDSA = argList[1]
                elif argList[0] == "eof":
                    argEOF = argList[1]
                elif argList[0] == "drr":
                    argDRR = argList[1]
                elif argList[0] == "addr":
                    argAddr = argList[1]
                elif argList[0] == "tag":
                    argTag = argList[1]
                elif argList[0] == "user":
                    argUser = argList[1]
                elif argList[0] == "cache":
                    argCache = argList[1]
                else:
                    printError(1, "Invalid key for dataMoverCommand")
                    exit(1)
        
        btt = extractNumber(argBTT)
        commandType = extractNumber(argCommandType) << 23
        dsa = extractNumber(argDSA) << 24
        eof = extractNumber(argEOF) << 30
        drr = extractNumber(argDRR) << 31
        addr = extractNumber(argAddr) << 32
        tag = extractNumber(argTag) << 64
        intVal = btt + commandType + dsa + eof + drr + addr + tag
        if len(packetArgs) == 10:
            xuser = extractNumber(argUser) << 72
            xcache = extractNumber(argCache) << 76
            intVal += xuser + xcache

    elif packetArgs[0] == "AMToken":
        assert(len(packetArgs) == 2 or len(packetArgs) == 3), "Invalid number of arguments for AMToken"

        if ":" not in packetArgs[1]:
            argToken = packetArgs[1]
            if len(packetArgs) == 3:
                argType = packetArgs[2]
            else:
                argType = "0"
        else:
            for arg in packetArgs[1:]:
                argList = arg.split(":")
                if argList[0] == "token":
                    argToken = argList[1]
                elif argList[0] == "type":
                    argType = argList[1]
                else:
                    printError(1, "Invalid key for AMToken")
                    exit(1)

        token = extractNumber(argToken) << 40
        AMtype = extractNumber(argType)
        intVal = token + AMtype
    else:
        return None

    return intVal
