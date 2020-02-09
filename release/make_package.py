#!/usr/bin/env python

import glob
from helpers import *

navigate_to_project_dir()

version_str = get_file_text(os.path.join('release', 'version.txt'))
printc('Create redistributable package version {}'.format(version_str), Colors.BOLD)


# when run with -v, *deployqt returns 1 and prints long help message,
# so don't print stdout and don't check return code
if IS_WINDOWS:
  check_qt_path(cmd = 'windeployqt -v', print_stdout = False, check_return_code = False)
if IS_MACOS:
  check_qt_path(cmd = 'macdeployqt -v', print_stdout = False, check_return_code = False)
if IS_LINUX:
  check_qt_path()


create_dir_if_none(OUT_DIR)
os.chdir(OUT_DIR)

recreate_dir_if_exists(REDIST_DIR)
os.chdir(REDIST_DIR)

package_name = PROJECT_NAME + '-' + version_str


########################################################################
#                             Windows

def make_package_for_windows():
  print_header('Run windeployqt...')
  execute('windeployqt ..\\..\\bin\\{} --dir . --no-translations --no-system-d3d-compiler --no-opengl-sw'.format(PROJECT_EXE))

  print_header('Clean some excessive files...')
  remove_files(['libEGL.dll', 'libGLESV2.dll'])
  remove_files_in_dir('sqldrivers', ['qsqlmysql.dll', 'qsqlodbc.dll', 'qsqlpsql.dll'])
  remove_files_in_dir('imageformats', ['qdds.dll', 'qicns.dll', 'qtga.dll', 'qtiff.dll', 'qwbmp.dll', 'qwebp.dll'])

  print_header('Copy project files...')
  copy_file('..\\..\\bin\\' + PROJECT_EXE, '.')

  # Seems sometimes windeployqt does copy these files,
  # but I definitely had cases when they had not been in the the place...
  print_header('Copy additional files ignored by windeployqt...')
  copy_files(find_qt_dir(), [
    'libgcc_s_seh-1.dll',  # doesn't exist in Qt 5.7
    'libstdc++-6.dll',
    'libwinpthread-1.dll',
    'libgcc_s_dw2-1.dll',  # for Qt 5.7
  ], '.')

  # TODO: copy samples

  print_header('Pack files to zip...')
  global package_name
  package_name = '{}-win-x{}.zip'.format(package_name, get_exe_bits(PROJECT_EXE))
  zip_dir('.', '..\\' + package_name)


########################################################################
#                           Linux

def make_package_for_linux():
  print('TODO')

########################################################################
#                              macOS

def make_package_for_macos():
  print('TODO')

########################################################################

if IS_WINDOWS:
  make_package_for_windows()
elif IS_LINUX:
  make_package_for_linux()
elif IS_MACOS:
  make_package_for_macos()

print('\nPackage created: {}'.format(package_name))
printc('Done\n', Colors.OKGREEN)
