import argparse
import random
import os
import string


def dir_path(string):
    if os.path.exists(string):
        return string
    else:
        raise NotADirectoryError(string)


print(os.getcwd())
parser = argparse.ArgumentParser()
parser.add_argument("--command", default="run", help="entrypoint command for the software (run/create/join/register)")
parser.add_argument("--ip", default="192.168.1.10", help="ip of the server")
parser.add_argument("--port", default="8081", help="port of the server")
parser.add_argument("--login", help="comma separated logins to the users login1,login2,login3")
parser.add_argument("--password", help="comma separated passwords to the users pass1,pass2,pass3")
parser.add_argument("--access-code", default="x", help="access code for joining session")
parser.add_argument("--exe-path", default="Client.exe", type=dir_path, help="path to Client.exe")

args = parser.parse_args()
passwords = args.password.split(",")
logins = args.login.split(",")
size = min(len(passwords), len(logins))
for i in range(size):
    filename = "".join(random.choice(string.ascii_letters) for _ in range(8))
    cmd = " ".join([args.exe_path, args.command, "--ip", args.ip, "--port", args.port, "--login", logins[i],
                    "--password", passwords[i], "--access-code", args.access_code, "--filename", filename])
    os.system("start cmd /k " + cmd)






