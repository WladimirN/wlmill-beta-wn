/*
WLToolOffset - ������� ������ ������ � ���������� ������������

���������:
 1.�������� ���� ���� (WLToolOffset.js) � ����� /wlmillconfig/script/include
 2.����������,  �������� � MScript.
   function init()
   {
   SCRIPT.includeFile("/include/WLToolOffset.js")
   ....	   
   }
 
01/09/2022 - ������ �����
*/

function WLToolOffsetInit()
{
TOOLBARTOOLS.addButton("WLToolOffsetBUTTON")	

WLToolOffsetBUTTON.setText("ToolOffset")
		 
WLToolOffsetBUTTON.clearMenu() 
WLToolOffsetBUTTON.addButtonMenu("X","WLToolOffsetSet('X')","������������� �������� �� X")
WLToolOffsetBUTTON.addButtonMenu("Y","WLToolOffsetSet('Y')","������������� �������� �� Y")
WLToolOffsetBUTTON.addButtonMenu("Z","WLToolOffsetSet('Z')","������������� �������� �� Z")

SCRIPT.console("WLToolOffsetInit()")
}

function WLToolOffsetSet(name)
{
var value

SCRIPT.console(name);

value=DIALOG.enterNum("������� ��������� "+name,MACHINE.getCurPositionSC(name))

if(DIALOG.isOk())
  MACHINE.setCurPositionSCT(name,value)

}
