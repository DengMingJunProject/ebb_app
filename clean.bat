@echo off
echo ɾ��astyle��ʱ�ļ�...
del /f /s /q *.pre

echo ɾ��IAR��ʱ�ļ�...
del /f /s /q *.o *.pbi *.cout *.pbd *.browse "Backup of*" 

echo ɾ��Keil��ʱ�ļ�...

del *.bak *.ddk  *.edk *.lst *.lnp *.mpf *.mpj *.obj *.omf *.plg *.rpt *.tmp *.__i *.crf *.o *.d  *.axf *.tra *.dep  JLinkLog.txt *.iex *.htm  *.sct  *.map *.sisc *.Bkp *.pre *.orig  *.adlio *._2i EventRecorderStub.scvd JLinkSettings.ini /s
echo �����ɣ�

echo. & pause