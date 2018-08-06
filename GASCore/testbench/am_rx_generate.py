import os
import json

def main():

    repo_path = os.environ.get('SHOAL_PATH')
    if repo_path is None:
        print("Error: SHOAL_PATH not defined in env")
        exit(-1)
    
    os.system("python $SHOAL_PATH/share/testbench/parse_json.py 0 /GASCore/testbench/am_rx.json")

    f = json.load(open(repo_path + "/GASCore/testbench/am_rx_out.json", "r"))

    testFileName = repo_path + "/GASCore/testbench/am_rx.dat"
    f_test = open(testFileName, "w+")

    for block in f['concurrent']:
        for packet in block['sequential']:
            for word in packet['payload']:
                f_test.write(packet['interface']+" "+'0x{0:0{1}X}'.format(word['data'], 8)+" "+
                str(word['last'])+" "+str(word['custom'])+"\n")

    f_test.close()

if __name__ == "__main__":
    main()