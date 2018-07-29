import re, sys
from subprocess import call

if len(sys.argv) != 2:
        print("Give name of the file")
        sys.exit(1)

incPattern = re.compile("\.INCLUDE: ")
tocPattern = re.compile(".TOC: here")
h2Pattern  = re.compile("<h2>.+</h2>")
h3Pattern  = re.compile("<h3>.+</h3>")
chapterCounter = 0;
ToC            = "<h2>Table of Contents</h2>\n"

# Build ToC

with open(sys.argv[1]) as f:
    for line in f:
        if h2Pattern.match(line.lstrip(' ')):
                chapterCounter = chapterCounter + 1
                ToC = ToC + "<a href=\"#" + str(chapterCounter) + "\">" + line.lstrip(' ')[4:-6] + "</a><br/>\n"

# Highlight and copy

chapterCounter = 0
sectionCounter = 0

with open(sys.argv[1]) as f:
    for line in f:
        if incPattern.match(line.lstrip(' ')):
            incf = line.lstrip(' ')[10:].replace('\n', '')
            call(["source-highlight", "-q", "-f", "html-css", incf])
            syncf = incf + ".html"
            with open(syncf, 'r') as myfile:
                filec = myfile.read();
                filec = filec.replace('<tt>', '')
                filec = filec.replace('</tt>', '')
                sys.stdout.write(filec)
        elif h2Pattern.match(line.lstrip(' ')):
                sectionCounter = 0
                chapterCounter = chapterCounter + 1
                newLine = "  <h2><a id=\"" + str(chapterCounter) + "\"></a>" + str(chapterCounter) + ". " + line.lstrip(' ')[4:]
                sys.stdout.write(newLine)
        elif h3Pattern.match(line.lstrip(' ')):
                sectionCounter = sectionCounter + 1
                newLine = "    <h3>" + str(chapterCounter) + "." + str(sectionCounter) + " " + line.lstrip(' ')[4:]
                sys.stdout.write(newLine)
        elif tocPattern.match(line.lstrip(' ')):
                sys.stdout.write(ToC)
        else:
            sys.stdout.write(line)
