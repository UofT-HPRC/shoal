import sys

### printMacro ###
# This function will print the value of a macro defined in a C/C++ 
# header file
# 
# Arguments:
#   header: absolute path to the header file containing the macro
#   macro: the name of the macro to print (case sensitive)
#
# Return: None
def printMacro(header, macro):
    import subprocess

    command = "g++ $SHOAL_PATH/share/src/print_macro.cpp -w \
        -I$SHOAL_PATH/share/include -I$SHOAL_VIVADO_HLS \
        -include " + header + " -D'MACRO_VALUE=" + \
        macro + "' -o $SHOAL_PATH/share/build/bin/print_macro"

    try:
        subprocess.check_output(command, shell=True)
    except subprocess.CalledProcessError as e:
        print("Error code", e.returncode, e.output)
        exit(1)
    
    subprocess.check_output("$SHOAL_PATH/share/build/bin/print_macro", shell=True)

### evalMacro ###
# This function will evaluate the value of a numeric macro defined in a C/C++ 
# header file and return it for use within the Python environment
# 
# Arguments:
#   header: absolute path to the header file containing the macro
#   macro: the name of the macro to evaluate (case sensitive)
#
# Return: integer value of the macro
def evalMacro(header, macro):
    import subprocess

    command = "g++ $SHOAL_PATH/share/src/eval_macro.cpp -w \
        -I$SHOAL_PATH/share/include -I$SHOAL_VIVADO_HLS \
        -include " + header + " -DMACRO_VALUE=" + \
        macro + " -o $SHOAL_PATH/share/build/bin/eval_macro"

    try:
        subprocess.check_output(command, shell=True)
    except subprocess.CalledProcessError as e:
        print("Error code", e.returncode, e.output)
        exit(1)
    
    try:
        subprocess.check_output("$SHOAL_PATH/share/build/bin/eval_macro", shell=True)
    except subprocess.CalledProcessError as e:
        return e.returncode
    else:
        return 0

if __name__ == "__main__":

    for arg in sys.argv:
        if arg == "-h" or arg == "--help":
            print("Usage: python share.py [function] [args...]")
            print("Functions: ")
            print("   evalMacro header macro, returns int")
            print("   printMacro header macro")
            exit(1)

    if (len(sys.argv) > 1):
        if sys.argv[1] == "evalMacro" and len(sys.argv) == 4:
            print(evalMacro(sys.argv[2], sys.argv[3]))
        elif sys.argv[1] == "printMacro" and len(sys.argv) == 4:
            print(printMacro(sys.argv[2], sys.argv[3]))
        else:
            print("Unknown flags. Use -h or --help")
    else:
        print("Needs flags. Use -h or --help")