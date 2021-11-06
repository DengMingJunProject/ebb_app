@echo off
echo 删除astyle临时文件...
del /f /s /q *.pre

echo 删除IAR临时文件...
del /f /s /q *.o *.pbi *.cout *.pbd *.browse "Backup of*" 

echo 删除Keil临时文件...

del *.bak *.ddk  *.edk *.lst *.lnp *.mpf *.mpj *.obj *.omf *.plg *.rpt *.tmp *.__i *.crf *.o *.d  *.axf *.tra *.dep  JLinkLog.txt *.iex *.htm  *.sct  *.map *.sisc *.Bkp *.pre *.orig  *.adlio *._2i EventRecorderStub.scvd JLinkSettings.ini /s
echo 清除完成！

echo. & pause