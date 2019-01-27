###############################################################################
# Copyright 2017 - 2019 www.octaspire.com                                     #
#                                                                             #
# Licensed under the Apache License, Version 2.0 (the "License");             #
# you may not use this file except in compliance with the License.            #
# You may obtain a copy of the License at                                     #
#                                                                             #
#  http://www.apache.org/licenses/LICENSE-2.0                                 #
#                                                                             #
# Unless required by applicable law or agreed to in writing, software         #
# distributed under the License is distributed on an "AS IS" BASIS,           #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.    #
# See the License for the specific language governing permissions and         #
# limitations under the License.                                              #
# --------------------------------------------------------------------------- #
# This python program is tested with Python 2.7.15 in Microsoft Windows 10.   #
#                                                                             #
# It first downloads, extracts and sets up all dependencies for building      #
# Octaspire Dern and all the plugins and examples in Windows. Then it builds  #
# Octaspire Dern and all the plugins and examples.                            #
#                                                                             #
# Required software: Python 2.7, git and Visual Studio 2017                   #
#                    (community edition).                                     #
#                                                                             #
# Usage: 1. Start 'Developer Command Prompt for VS 2017'.                     #
#        2. git clone https://github.com/octaspire/dern.git                   #
#        3. cd dern\release                                                   #
#        4. python how-to-build\windows-msvc.py                               #
###############################################################################
import getopt
import os
import re
import shutil
import sys
import urllib
import zipfile

###############################################################################
# Helper functions                                                            #
###############################################################################
def octaspire_copydir(p):
    sourceFiles = os.listdir(p)
    for f in sourceFiles:
        shutil.copy(p + '/' + f, '.')

def octaspire_depend(prefix, f, dirName, copyFromPath):
    print('- Dependency {}'.format(dirName))
    if not os.path.isfile(f):
        print('\tDownloading {}'.format(f))
        urllib.urlretrieve(prefix + f, f)
    if not os.path.isdir(dirName):
        print('\tExtracting {}'.format(f))
        zip_file = zipfile.ZipFile(f, 'r')
        zip_file.extractall()
        zip_file.close()
    if len(copyFromPath) > 0:
        print('\tCopying from {}'.format(copyFromPath))
        octaspire_copydir(copyFromPath)
    print('')

def octaspire_depend_sourceforge(url, f, dirName, copyFromPath):
    print('- Dependency {}'.format(dirName))
    if not os.path.isfile(f):
        print('\tDownloading {}'.format(f))
        urllib.urlretrieve(url, f)
    if not os.path.isdir(dirName):
        print('\tExtracting {}'.format(f))
        zip_file = zipfile.ZipFile(f, 'r')
        zip_file.extractall(dirName)
        zip_file.close()
    if len(copyFromPath) > 0:
        print('\tCopying from {}'.format(copyFromPath))
        octaspire_copydir(copyFromPath)
    print('')

def octaspire_build(resultName, desc, buildCommand, *runExamples):
    print('- Building   {}\n\t     {}'.format(resultName, desc))
    print('-----------------------------------------------------')
    os.system(buildCommand)
    print('-----------------------------------------------------')
    if not os.path.isfile(resultName):
        print('\t ERROR {} FAILED TO BUILD'.format(resultName))
        sys.exit(1)
    for a in runExamples:
        print('             Run or test like this: {}'.format(a))
    print('')

def octaspire_create_lib_file(dllFileName):
    defFileName = dllFileName.replace('.dll', '.def')
    tmpFileName = dllFileName.replace('.dll', '.tmp')
    libFileName = dllFileName.replace('.dll', '.lib')
    print('- Generating .lib for {}'.format(dllFileName))
    print('-----------------------------------------------------')
    os.system('dumpbin /exports {} /OUT:{}'.format(dllFileName, tmpFileName))
    ordinalSeen = False
    defFile = open(defFileName, 'w')
    defFile.write('EXPORTS\n')
    for line in open(tmpFileName):
        if ordinalSeen:
            if 'Summary' in line:
                break
            else:
                columns = line.split()
                if len(columns) >= 4:
                    defFile.write('{}\n'.format(columns[3]))
        elif re.match(r'.*ordinal.*hint.*RVA.*name.*', line):
            ordinalSeen = True
    defFile.close()
    os.system('lib /def:{} /OUT:{}'.format(defFileName, libFileName))
    print('-----------------------------------------------------')

def octaspire_usage():
    print('\nUsage: python {} [OPTION]\n'.format(os.path.basename(__file__)))
    print('where optional options are:')
    print('-g - Build with Debug information.')
    print('-h - Show help and exit.')

###############################################################################
# main function                                                               #
###############################################################################
def main(argv):
    cl             = 'cl /nologo /W2 '
    cflags         = '/DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS '
    cflagsChipmunk = '/DNDEBUG '
    try:
        opts, args = getopt.getopt(argv, 'g')
    except getopt.GetoptError:
        octaspire_usage()
        sys.exit(1)
    for opt, arg in opts:
        if opt == '-g':
            cflags += '/DEBUG /Z7 '
            cflagsChipmunk = ''
            print('- Building in debug configuration.\n')
        elif opt == '-h':
            octaspire_usage()
            sys.exit(0)
    ###########################################################################
    # Get dependencies and create .lib files                                  #
    ###########################################################################
    # Get dependency: PDCurses library
    octaspire_depend_sourceforge(
        'https://sourceforge.net/projects/pdcurses/files/pdcurses/3.4/pdc34dllw.zip/download',
        'pdc34dllw.zip',
        'pdc34dllw',
        'pdc34dllw')
    # Get dependency: SDL2
    octaspire_depend(
        'https://www.libsdl.org/release/',
        'SDL2-devel-2.0.9-VC.zip',
        'SDL2-2.0.9',
        'SDL2-2.0.9/lib/x86/')
    # Get dependency: SDL2_ttf libraries
    octaspire_depend(
        'https://www.libsdl.org/projects/SDL_ttf/release/',
        'SDL2_ttf-2.0.14-win32-x86.zip',
        'SDL2_ttf-2.0.14-win32-x86',
        '')
    octaspire_create_lib_file('SDL2_image.dll')
    octaspire_create_lib_file('SDL2_mixer.dll')
    octaspire_create_lib_file('SDL2_ttf.dll')
    # Get dependency: SDL2_ttf headers
    octaspire_depend(
        'https://www.libsdl.org/projects/SDL_ttf/release/',
        'SDL2_ttf-2.0.14.zip',
        'SDL2_ttf-2.0.14',
        '')
    # Get dependency: SDL2_mixer libraries
    octaspire_depend(
        'https://www.libsdl.org/projects/SDL_mixer/release/',
        'SDL2_mixer-2.0.4-win32-x86.zip',
        'SDL2_mixer-2.0.4-win32-x86',
        '')
    # Get dependency: SDL2_mixer headers
    octaspire_depend(
        'https://www.libsdl.org/projects/SDL_mixer/release/',
        'SDL2_mixer-2.0.4.zip',
        'SDL2_mixer-2.0.4',
        '')
    # Get dependency: SDL2_image libraries
    octaspire_depend(
        'https://www.libsdl.org/projects/SDL_image/release/',
        'SDL2_image-2.0.4-win32-x86.zip',
        'SDL2_image-2.0.4-win32-x86',
        '')
    # Get dependency: SDL2_image headers
    octaspire_depend(
        'https://www.libsdl.org/projects/SDL_image/release/',
        'SDL2_image-2.0.4.zip',
        'SDL2_image-2.0.4',
        '')
    ###########################################################################
    # Build programs, plugins and examples                                    #
    ###########################################################################
    # Build Unit test runner
    octaspire_build(
        'octaspire-dern-unit-test-runner.exe',
        'stand alone unit test runner',
        cl + cflags + '/DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION octaspire-dern-amalgamated.c /link /out:octaspire-dern-unit-test-runner.exe',
        'octaspire-dern-unit-test-runner.exe')
    # Build interactive Dern REPL
    octaspire_build(
        'octaspire-dern-repl.exe',
        'interactive Dern REPL',
        cl + cflags + '/DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION octaspire-dern-amalgamated.c /link /out:octaspire-dern-repl.exe',
        'octaspire-dern-repl.exe')
    # Build Dern ncurses plugin
    octaspire_build(
        'libdern_ncurses.dll',
        'Dern ncurses plugin (uses pdcurses)',
        cl + cflags + '/I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_ncurses.c /link pdcurses.lib /out:libdern_ncurses.dll',
        'octaspire-dern-repl.exe examples\dern-ncurses-example.dern')
    # Build Dern SDL2 plugin
    octaspire_build(
        'libdern_sdl2.dll',
        'Dern SDL2 plugin (uses SDL2, SDL2_image, SDL2_mixer and SDL2_ttf)',
        cl + cflags + '/I. /ISDL2-2.0.9\include /ISDL2_image-2.0.4 /ISDL2_mixer-2.0.4 /ISDL2_ttf-2.0.14 /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY /Dmain=SDL_main /LD plugins\dern_sdl2.c /link SDL2.lib SDL2_mixer.lib SDL2_ttf.lib SDL2_image.lib opengl32.lib glu32.lib /out:libdern_sdl2.dll',
        'octaspire-dern-repl.exe examples\dern-sdl2-example.dern')
    # Build Dern Nuklear plugin
    octaspire_build(
        'libdern_nuklear.dll',
        'Dern Nuklear plugin',
        cl + cflags + '/I. /Iplugins/external/nuklear /ISDL2-2.0.9\include /ISDL2_image-2.0.4 /ISDL2_mixer-2.0.4 /ISDL2_ttf-2.0.14 /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY /DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY /LD plugins\dern_nuklear.c /link SDL2.lib SDL2_mixer.lib SDL2_ttf.lib SDL2_image.lib opengl32.lib glu32.lib /out:libdern_nuklear.dll',
        'octaspire-dern-repl.exe examples\dern-nuklear-example.dern')
    # Build Dern socket plugin
    octaspire_build(
        'libdern_socket.dll',
        'Dern socket plugin',
        cl + cflags + '/I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_socket.c /link ws2_32.lib /out:libdern_socket.dll',
        'octaspire-dern-repl.exe -I examples examples\irc-client-ncurses.dern',
        'octaspire-dern-repl.exe -I examples examples\irc-client-nuklear.dern')
    # Build Dern easing plugin
    octaspire_build(
        'libdern_easing.dll',
        'Dern easing plugin',
        cl + cflags + '/I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_easing.c /link /out:libdern_easing.dll',
        'octaspire-dern-repl.exe examples\dern-easing-example.dern')
    # Build Dern animation plugin
    octaspire_build(
        'libdern_animation.dll',
        'Dern animation plugin',
        cl + cflags + '/I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_animation.c /link /out:libdern_animation.dll',
        'octaspire-dern-repl.exe examples\dern-animation-example.dern')
    # Build Dern dir plugin
    octaspire_build(
        'libdern_dir.dll',
        'Dern directory plugin',
        cl + cflags + '/W2 /I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_dir.c /link /out:libdern_dir.dll',
        'octaspire-dern-repl.exe examples\dern-dir-example.dern')
    print('Building Chipmunk library...')
    os.system(cl + cflagsChipmunk + '/Iplugins/external/chipmunk/include /Iplugins/external/chipmunk/include/chipmunk /LD plugins/external/chipmunk/src/*.c /link /out:libchipmunk.dll')
    # Build Dern chipmunk plugin
    octaspire_build(
        'libdern_chipmunk.dll',
        'Dern chipmunk plugin',
        cl + cflags + '/I. /Iplugins/external/chipmunk/include/ /Iplugins/external/chipmunk/include/chipmunk /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_chipmunk.c plugins\external\sqlite3\sqlite3.c /link chipmunk.lib /out:libdern_chipmunk.dll',
        'octaspire-dern-repl.exe examples\dern-chipmunk-example.dern')
    #print('Compiling SQLite3...')
    #os.system(cl + cflags + '/Iplugins/external/sqlite3 plugins/external/sqlite3/sqlite3.c')
    # Build Dern SQLite3 plugin
    octaspire_build(
        'libdern_sqlite3.dll',
        'Dern SQLite3 plugin',
        cl + cflags + '/I. /Iplugins/external/sqlite3/ /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\external\sqlite3\sqlite3.c plugins\dern_sqlite3.c /link /out:libdern_sqlite3.dll',
        'octaspire-dern-repl.exe examples\dern-sqlite3-example.dern')
    # Build embedding example
    octaspire_build(
        'embedding-example.exe',
        'embedding example',
        cl + cflags + '/I. /DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS examples\embedding-example.c /link /out:embedding-example.exe',
        'embedding-example.exe')
    # Build binary library example
    octaspire_build(
        'libmylib.dll',
        'binary library example',
        cl + cflags + '/I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD examples\mylib.c /link /out:libmylib.dll',
        'octaspire-dern-repl.exe examples\use-mylib.dern')

if __name__ == '__main__':
    main(sys.argv[1:])
