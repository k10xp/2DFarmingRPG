# run this to populate the sha and build time defines in the WfVersion.h file, only run it in CI.
# Written as a python script so it should hopefully work on windows 
import subprocess
import time

def get_git_sha():
    return subprocess.check_output(["git", "log", "-1", "--format=%H"]).decode().strip()

def main():
    data = ""
    with open('./include/WfVersion.h', 'r') as file:
        data = file.read()
        sha = get_git_sha()
        print(f"sha: {sha}")
        formatted_time = time.strftime("%x-%X", time.gmtime())
        print(f"time: {formatted_time}")
        data = data.replace("#define BUILD_SHA \"<local_build>\"", f"#define BUILD_SHA \"{sha}\"")
        data = data.replace("BUILD_TIME \"<local_build>\"", f"BUILD_TIME \"{formatted_time}\"")
    with open("./include/WfVersion.h", "w") as file:
        file.write(data)

main()