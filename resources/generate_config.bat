@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

set /p input="请输入WIFI的名称(仅支持2.4Ghz): "
if "%input:~-1%"==" " set input=%input:~0,-1%
set /p =%input%<nul >>temp.txt
certutil -encode -f temp.txt encoded.txt

set lineFlag=0
for /f "delims=*" %%i in (encoded.txt) do (
set /a lineFlag+=1
  if !lineFlag!==2 (
    set ssid=%%i 
  )
)
if "%ssid:~-1%"==" " set ssid=%ssid:~0,-1%
echo 成功：wifi名加密为: !ssid!
del temp.txt
del encoded.txt

set /p input="请输入WIFI的密码: "
set /p =%input%<nul >>temp.txt
certutil -encode -f temp.txt encoded.txt


set lineFlag=0
for /f "delims=*" %%i in (encoded.txt) do (
set /a lineFlag+=1
  if !lineFlag!==2 (
    set wifipwd=%%i 
  )
)

if "%wifipwd:~-1%"==" " set wifipwd=%wifipwd:~0,-1%
echo 成功：wifi密码加密为: !wifipwd!
del temp.txt
del encoded.txt

set /p input="请输入拓竹账户的电话号码: "
set /p =%input%<nul >>temp.txt
certutil -encode -f temp.txt encoded.txt

set lineFlag=0
for /f "delims=*" %%i in (encoded.txt) do (
set /a lineFlag+=1
  if !lineFlag!==2 (
    set bblphone=%%i 
  )
)
if "%bblphone:~-1%"==" " set bblphone=%bblphone:~0,-1%
echo 成功：电话号码加密为: !bblphone!
del temp.txt
del encoded.txt

set /p input="请输入拓竹密码: "
set /p =%input%<nul >>temp.txt
certutil -encode -f temp.txt encoded.txt


set lineFlag=0
for /f "delims=*" %%i in (encoded.txt) do (
set /a lineFlag+=1
  if !lineFlag!==2 (
    set bblpwd=%%i 
    echo !bblpwd!
  )
)

if "%bblpwd:~-1%"==" " set bblpwd=%bblpwd:~0,-1%
echo 成功：拓竹密码加密为: !bblpwd!
del temp.txt
del encoded.txt


set /p input="请输入拓竹用户id（一串纯数字，官网登陆后你的用户名里的）: "
set /p =%input%<nul >>temp.txt
certutil -encode -f temp.txt encoded.txt


set lineFlag=0
for /f "delims=*" %%i in (encoded.txt) do (
set /a lineFlag+=1
  if !lineFlag!==2 (
    set bbluserid=%%i 
    echo !bbluserid!
  )
)

if "%bbluserid:~-1%"==" " set bbluserid=%bbluserid:~0,-1%
echo 成功：拓竹用户id加密为: !bbluserid!
del temp.txt
del encoded.txt


:: JSON内容
set "jsonContent={^"ssid^": ^"!ssid!^",^"pwd^": ^"!wifipwd!^",^"cloud-email^": ^"!bblphone!^",^"cloud-password^": ^"!bblpwd!^",^"cloud-region^": ^"China^",^"user-id^": ^"!bbluserid!^"}"



:: 创建test.json文件
rmdir /S /q pack_dir
echo Folder "pack_dir" was deleted.
mkdir pack_dir
(
echo %jsonContent%
) > ./pack_dir/config.json

:: 完成提示
echo config.json文件已创建。
pause
endlocal
