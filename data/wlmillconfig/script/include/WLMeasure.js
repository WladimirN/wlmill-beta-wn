/*
WLMeasure - ������� ������ ������ � ���������� ������������

���������:
 1.�������� ���� ���� (WLMeasure.js) � ����� /wlmillconfig/script/include
 2.����������,  �������� � MScript.
   function init()
   {
   SCRIPT.includeFile("/include/WLMeasure.js")
   ....	   
   }
 
01/09/2022 - ������ �����
*/

function WLMeasureInit()
{
TOOLBARTOOLS.addButton("WLMEASUREBUTTON")	

WLMEASUREBUTTON.setText("Measure")
		 
WLMEASUREBUTTON.clearMenu() 
WLMEASUREBUTTON.addButtonMenu("X","WLMeasureSet('X')","������������� �������� �� X")
WLMEASUREBUTTON.addButtonMenu("Y","WLMeasureSet('Y')","������������� �������� �� Y")
WLMEASUREBUTTON.addButtonMenu("Z","WLMeasureSet('Z')","������������� �������� �� Z")

SCRIPT.console("WLMeasureInit()")
}

function WLMeasureSet(name)
{
var value

SCRIPT.console(name);

value=DIALOG.enterNum("������� ��������� "+name,MACHINE.getCurPositionSC(name))

if(DIALOG.isOk())
  MACHINE.setCurPositionSCT(name,value)

}
