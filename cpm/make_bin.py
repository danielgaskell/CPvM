from pathlib import Path

data = Path("tpa.bin").read_bytes()

outstring = "void wrap_beg(void) __naked {\n"
outstring += "\t__asm\n"
outstring += "\t\t.db " + ",".join([str(char) for char in data]) + "\n"
outstring += "\t__endasm;\n}"

with open("tpa.h", "w") as file:
    file.write(outstring)