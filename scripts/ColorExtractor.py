import os

colortable = {}

def read():
    fileData = open(input("Enter color table text file path with file name: "), "r")
    fileContents = fileData.read()

    colors = fileContents.splitlines()

    for c in colors:
        colora = c.split("=")
        colorname = colora[0].rstrip().lstrip().upper()
        colorvalues = colora[1].lstrip().rstrip()
        colortable[colorname] = colorvalues

def createfile(file):
    fileData = open(file, "w+")

    fileHeader = file.replace(".h", "").upper()
    fileData.write("#ifndef %s\n" % fileHeader)

    fileData.write("\n")
    fileData.write("struct _colors\n")
    fileData.write("{\n")

    for x in colortable:  
        fileData.write("    vec3 %s = " % x)
        fileData.write("%s\n" % colortable[x])


    fileData.write("}static colors;\n")

    fileData.write("#define %s\n" % fileHeader)
    fileData.write("#endif // %s\n" % fileHeader)


read()
createfile(input("Enter the exported filename: "))
    
