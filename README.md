# duplicatefile

command format: dir dir dir ... {-strict} {-h} {-backup dir}
-h                : print help info
-quick(default)   : use an evenly distributed 128byte to determine if the files are consistent
-strict           : use strict mode, if quick mark is same, will use md5 to compare
-extremely-strict : if md5 is same, direct compare the binary data
-backup           : do not delete the duplicate file, only move to backup dirs
all dir input will used to remove duplicate files

