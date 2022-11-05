/*
WLMeasure - базовый скрипт работы с смещениями инструментом

Установка:
 1.Копируем этот файл (WLMeasure.js) в папку /wlmillconfig/script/include
 2.Подключаем,  добавляя в MScript.
   function init()
   {
   SCRIPT.includeFile("/include/WLMeasure.js")
   ....	   
   }
 
01/09/2022 - первый релиз
*/

function WLMeasureInit()
{
TOOLBARTOOLS.addButton("WLMEASUREBUTTON")	

WLMEASUREBUTTON.setText("Measure")
		 
WLMEASUREBUTTON.clearMenu() 
WLMEASUREBUTTON.addButtonMenu("X","WLMeasureSet('X')","Корректировка смещения по X")
WLMEASUREBUTTON.addButtonMenu("Y","WLMeasureSet('Y')","Корректировка смещения по Y")
WLMEASUREBUTTON.addButtonMenu("Z","WLMeasureSet('Z')","Корректировка смещения по Z")

SCRIPT.console("WLMeasureInit()")
}

function WLMeasureSet(name)
{
var value

SCRIPT.console(name);

value=DIALOG.enterNum("Введите положение "+name,MACHINE.getCurPositionSC(name))

if(DIALOG.isOk())
  MACHINE.setCurPositionSCT(name,value)

}
