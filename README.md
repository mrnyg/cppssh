To get cppssh and all dependant software:

repo init -u https://github.com/cdesjardins/cppsshManifest.git 
repo sync

Then run the buildall.sh script on linux, similar steps can be used on windows, but an automated script for windows is still in the works.

cppssh uses the Botan library, version 1.11<br>
https://github.com/randombit/botan <br>
http://botan.randombit.net/ <br>

To build just botan:

Windows:
```
buildbotan.bat
```

Linux:
```
./buildbotan.sh
```


