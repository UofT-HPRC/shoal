import os
import json
import sys

def generate(filename):

    repo_path = os.environ.get('SHOAL_PATH')
    if repo_path is None:
        print("Error: SHOAL_PATH not defined in env")
        exit(-1)

    workingDirectory = repo_path + "/GASCore/testbench/"
    
    os.system("python $SHOAL_PATH/share/testbench/parse_json.py 1 " + \
        workingDirectory + filename + ".json")

    f = json.load(open(workingDirectory + filename + "_out.json", "r"))

    testFileName = workingDirectory + filename + ".dat"
    f_test = open(testFileName, "w+")

    for block in f['concurrent']:
        for packet in block['sequential']:
            for word in packet['payload']:
                f_test.write(packet['interface']+ " " + \
                    '0x{0:0{1}X}'.format(word['data'], 8) + " " + \
                    str(word['last']) + " " + str(word['custom']) + " " + \
                    str(word['keep']) + " " + str(word['id'])+"\n")

    f_test.close()

if __name__ == "__main__":
    for arg in sys.argv:
        if arg == "-h" or arg == "--help":
            print("Usage: python generate.py filename")
            print("Arguments: ")
            print("   filename - name of the file to parse")
            exit(1)

    if (len(sys.argv) == 2):
        generate(sys.argv[1])
    else:
        print("Incorrect number of arguments. Use -h or --help")