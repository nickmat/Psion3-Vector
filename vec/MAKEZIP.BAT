@echo off

REM remove old files
del release\*.txt
del release\app\*.app
del release\app\vector\*.v??
del release\vec\*.vec
del release\wrd\*.wrd

REM position new files
copy readme.txt   release
copy history.txt  release
copy vector.app   release\app
copy logic.vsl    release\app\vector
copy electric.vsl release\app\vector
copy shapes.vsl   release\app\vector
copy ascii.vsl    release\app\vector
copy traction.vec release\vec
copy jkff.vec     release\vec
copy vpmanual.wrd release\wrd

REM create zip
del %1.zip
cd release
zip ..\%1.zip -R *.*
cd ..
