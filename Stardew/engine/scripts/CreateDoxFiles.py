import os

# wrap the markdown docs in this .dox bullshit
# a custom script shouldn't be necesary for this but i can't get the \include feature of doxygen to work

template = """

/** \\page <<PAGE_NAME>>
 
<<PAGE_CONTENT>>
 
*/

"""

def main():
    for file in os.listdir("../docs"):
        path = os.path.join("../docs", file)
        outPath = os.path.join("../src", file).replace(".md", ".dox")
        print(path)
        r = ""
        with open(path, "r") as f:
            md = f.read()
            r = template.replace("<<PAGE_NAME>>", file[:-3])
            r = r.replace("<<PAGE_CONTENT>>", md)
        with open(outPath, "w") as f:
            f.write(r)
    pass

main()