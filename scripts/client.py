import argparse
import random
import os
import string


def dir_path(string):
    if os.path.exists(string):
        return string
    else:
        raise FileNotFoundError(string)


print(os.getcwd())
parser = argparse.ArgumentParser()
parser.add_argument("--command", default="", help="Entrypoint command for the software (run/create/join/register). Default: ''")
parser.add_argument("--ip", default="127.0.0.1", help="IP of the server. Default: '127.0.0.1'")
parser.add_argument("--port", default="8081", help="Port of the server. Default: '8081'")
parser.add_argument("--login", help="Comma separated logins to the users login1,login2,login3")
parser.add_argument("--password", help="Comma separated passwords to the users pass1,pass2,pass3")
parser.add_argument("--access-code", default="", help="Access code for joining session")
parser.add_argument("--exe-path", default="./ClientMain.exe", type=dir_path, help="Path to client executable. May be ClientMain.exe or RandomClient.exe. Default: './ClientMain.exe'")

try:
    args = parser.parse_args()
except FileNotFoundError as file:
    print(f"\nERROR: Cannot find provided file: {file}.\n")
    parser.print_help()
    exit(1)
passwords = args.password.split(",")
logins = args.login.split(",")
size = min(len(passwords), len(logins))
for i in range(size):
    filename = "".join(random.choice(string.ascii_letters) for _ in range(8))
    cmd = " ".join([args.exe_path, args.command, "--ip", args.ip, "--port", args.port, "--login", logins[i],
                    "--password", passwords[i], "--access-code", args.access_code, "--filename", filename])
    os.system("start cmd /k " + cmd)






