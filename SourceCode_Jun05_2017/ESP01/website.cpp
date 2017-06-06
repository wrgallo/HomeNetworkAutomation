#include "./website.h"

void buildWebsite(){
  buildJavascript();
  webSite="<!DOCTYPE HTML>\n";
  webSite+="<title>Home Automation Network</title>\n";
  webSite+=javaScript;

  /* HTML PAGE, WITH THE HOME NETWORK AUTOMATION INFORMATION */
  webSite+="<style>\n";
  webSite+="        body {\n";
  webSite+="            height: 420px;\n";
  webSite+="            margin: 0 auto;\n";
  webSite+="            font-family: Courier New, Courier, monospace;\n";
  webSite+="            font-size: 18px;\n";
  webSite+="            text-align: center;\n";
  webSite+="            color: lightcyan;\n";
  webSite+="            background-color: darkslategray;\n";
  webSite+="            background-color: background-image: -webkit-gradient(linear, left top, left bottom, from(darkgray), to(darkslategray));\n";
  webSite+="            background-image: -webkit-linear-gradient(top, darkgray, darkslategray);\n";
  webSite+="            background-image: -moz-linear-gradient(top, darkgray, darkslategray);\n";
  webSite+="            background-image: linear-gradient(to bottom, darkgray, darkslategray);\n";
  webSite+="            background-repeat: no-repeat;\n";
  webSite+="        }\n";
  webSite+= "      p {\n";
  webSite+= "         text-align: center;\n";
  webSite+= "         font-size: 10px;\n";
  webSite+= "         font-style: italic;\n";
  webSite+= "         width: 100%;\n";
  webSite+= "      }\n";
  webSite+="        h1{\n";
  webSite+="           text-align: center;\n";
  webSite+="           font-size: 24px;\n";
  webSite+="           width: 100%;\n";
  webSite+="        }\n";
  webSite+="</style>\n";
  webSite+="<BODY onload='process()'>\n";
  webSite+="  <h1> Home Automation Network </h1>\n";
  webSite+="  <br><br>\n";
  
  webSite+="<table align=center width='600px' border='1' edgecolor='gray' bgcolor='darkslategray'>\n";
  webSite+="        <tbody>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left width='220px'>Lamp State</td>\n";
  webSite+="                <td align=center > <A id='var01'> </A></td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left>Motion Sensor State</td>\n";
  webSite+="                <td align=center> <A id='var02'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left>Buzzer State</td>\n";
  webSite+="                <td align=center> <A id='var03'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Time </td>\n";
  webSite+="                <td align=center> <A id='var04'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Date (DD/MM/YY) </td>\n";
  webSite+="                <td align=center> <A id='var05'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Weekday </td>\n";
  webSite+="                <td align=center> <A id='var06'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Alarm Time </td>\n";
  webSite+="                <td align=center> <A id='var07'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Alarm Day </td>\n";
  webSite+="                <td align=center> <A id='var08'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Alarm Set </td>\n";
  webSite+="                <td align=center> <A id='var09'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Night Time Start </td>\n";
  webSite+="                <td align=center> <A id='var10'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Night Time End </td>\n";
  webSite+="                <td align=center> <A id='var11'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="            <tr>\n";
  webSite+="                <td align=left> Night Time Set </td>\n";
  webSite+="                <td align=center> <A id='var12'></A> </td>\n";
  webSite+="            </tr>\n";
  webSite+="        </tbody>\n";
  webSite+="    </table>\n";

  webSite+="<br><table width=600px align='center'>\n";
  webSite+="    <a href='/?buttonChangeLampState=YES'><input name='buttonChangeLampState'   type='submit' class='button_3' value='Change Lamp State'   ></a>\n";
  webSite+="    &nbsp<a href='/?buttonChangeBuzzerState=YES'><input name='buttonChangeBuzzerState' type='submit' class='button_2' value=' Change Buzzer State  ' ></a>\n";
  webSite+="    &nbsp<a href='/?buttonChangeAlarmSet=YES'><input name='buttonChangeAlarmSet' type='submit' class='button_2' value=' Change Alarm Set ' ></a>\n";
  webSite+="    &nbsp<a href='/?buttonChangeNightTimeSet=YES'><input name='buttonChangeNightTimeSet' type='submit' class='button_2' value='Change Night Time Set' ></a>\n";
  webSite+="    </table>\n";
  
  webSite+="    <br><table width=600px align='center' border='0' edgecolor='gray' bgcolor='darkslategray'>\n";
  webSite+="        <tr><form action='/setTime' method='POST'>\n";
  webSite+="            <td align='left'>Time: </td>\n";
  webSite+="            <td align='center' width=220px>\n";
  webSite+="                <input name='timeHour' id='timeHour' type='number' min='0' max='23' class='setTimeForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  webSite+="                <input name='timeMin'  id='timeMin'  type='number' min='0' max='59' class='setTimeForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  webSite+="            </td><td align='center'>\n";
  webSite+="                <input name='buttonSetTime' type='submit' class='button_2' value='Set Time' />\n";
  webSite+="            </td>\n";
  webSite+="        </form></tr>\n";
  
  webSite+="        <tr><form action='/setDate' method='POST'>\n";
  webSite+="        <td align='left'>Date: </td>\n";
  webSite+="        <td align='center'>\n";
  webSite+="            <input name='dateDay'  id='dateDay' type='number' min='1'  max='31' class='setDateForm' placeholder='Day'   style='width:55px; text-align: center' required/>\n";
  webSite+="            <input name='dateMon'  id='dateMon' type='number' min='1'  max='12' class='setDateForm' placeholder='Month' style='width:55px; text-align: center' required/>\n";
  webSite+="            <input name='dateYea'  id='dateYea' type='number' min='17' max='50' class='setDateForm' placeholder='Year'  style='width:55px; text-align: center' required/>    \n";
  webSite+="        </td><td align='center'>\n";
  webSite+="            <input name='buttonSetDate' type='submit' class='button_2' value='Set Date' />\n";
  webSite+="        </td>\n";
  webSite+="        </form></tr>\n";
  
  webSite+="        <tr><form action='/setWeekday' method='POST'>\n";
  webSite+="            <td align='left'>Weekday: </td>\n";
  webSite+="            <td align='center'>\n";
  webSite+="                <input name='weekday'  id='weekday' type='number' min='0'  max='6' class='setWeekdayForm' placeholder='0 = Sunday ... 6 = Saturday'    style='width: 195px; text-align: center' required/>\n";
  webSite+="            </td><td align='center'>\n";
  webSite+="                <input name='buttonSetWeekday' type='submit' class='button_2' value='Set Weekday' />\n";
  webSite+="            </td>\n";
  webSite+="        </form></tr>\n";
  
  webSite+="        <tr><form action='/setAlarmTime' method='POST'>\n";
  webSite+="            <td align='left'>Alarm Time: </td>\n";
  webSite+="            <td align='center'>\n";
  webSite+="                <input name='alarmHour' id='alarmHour' type='number' min='0' max='23' class='setAlarmForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  webSite+="                <input name='alarmMin'  id='alarmMin'  type='number' min='0' max='59' class='setAlarmForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  webSite+="            </td><td align='center'>\n";
  webSite+="                <input name='buttonSetAlarmTime' type='submit' class='button_2' value='Set Alarm Time' />\n";
  webSite+="            </td>\n";
  webSite+="        </form></tr>\n";
  
  webSite+="        <tr><form action='/setAlarmDay' method='POST'>\n";
  webSite+="            <td align='left'>Alarm Day: </td>\n";
  webSite+="            <td align='center'>\n";
  webSite+="                <input name='alarmDay'  id='alarmDay' type='number' min='0'  max='6' class='setAlarmDayForm' placeholder='0 = Sunday ... 6 = Saturday'    style='width: 195px; text-align: center' required/>\n";
  webSite+="            </td><td align='center'>\n";
  webSite+="                <input name='buttonSetAlarmDay' type='submit' class='button_2' value='Set Alarm Day' />\n";
  webSite+="            </td>\n";
  webSite+="        </form></tr>\n";
  
  webSite+="        <tr><form action='/setNightTimeStart' method='POST'>\n";
  webSite+="            <td align='left'>Night Time Start: </td>\n";
  webSite+="            <td align='center'>\n";
  webSite+="                <input name='nightTimeHourStart' id='nightTimeHourStart' type='number' min='0' max='23' class='setNightTimeStartForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  webSite+="                <input name='nightTimeMinStart'  id='nightTimeMinStart'  type='number' min='0' max='59' class='setNightTimeStartForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  webSite+="            </td><td align='center'>\n";
  webSite+="                <input name='buttonsetNightTimeStart' type='submit' class='button_2' value='Set Night Time Start' />\n";
  webSite+="            </td>\n";
  webSite+="        </form></tr>\n";
  
  webSite+="        <tr><form action='/setNightTimeEnd' method='POST'>\n";
  webSite+="            <td align='left'>Night Time End: </td>\n";
  webSite+="            <td align='center'>\n";
  webSite+="                <input name='nightTimeHourEnd' id='nightTimeHourEnd' type='number' min='0' max='23' class='setNightTimeEndForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  webSite+="                <input name='nightTimeMinEnd'  id='nightTimeMinEnd'  type='number' min='0' max='59' class='setNightTimeEndForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  webSite+="            </td><td align='center'>\n";
  webSite+="                <input name='buttonsetNightTimeEnd' type='submit' class='button_2' value='Set Night Time End' />\n";
  webSite+="            </td>\n";
  webSite+="        </form></tr>\n";
  webSite+="    </table>\n";

  webSite+="  <br><br>You can access this page until you <a style='color: lightcyan;' href='/login?DISCONNECT=YES'>disconnect</a>\n";

  webSite += "  <table height='100px'></table>\n";
  webSite += "  <p>\n";
  webSite += "  <br>Wellington Rodrigo Gallo\n";
  webSite += "  <br><a href='mailto:w.r.gallo@grad.ufsc.br' style='color: lightcyan;'>w.r.gallo@grad.ufsc.br</a>\n";
  webSite += "  <br>2017\n";
  webSite += "  </p>\n";
  
  webSite+="</BODY>\n";
  webSite+="</HTML>\n";
}

void buildJavascript(){
  javaScript="<SCRIPT>\n";
  javaScript+="var xmlHttp=createXmlHttpObject();\n";

  javaScript+="function createXmlHttpObject(){\n";
  javaScript+=" if(window.XMLHttpRequest){\n";
  javaScript+="    xmlHttp=new XMLHttpRequest();\n";
  javaScript+=" }else{\n";
  javaScript+="    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  javaScript+=" }\n";
  javaScript+=" return xmlHttp;\n";
  javaScript+="}\n";

  javaScript+="function process(){\n";
  javaScript+=" if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n";
  javaScript+="   xmlHttp.open('PUT','xml',true);\n";
  javaScript+="   xmlHttp.onreadystatechange=handleServerResponse;\n"; // no brackets?????
  javaScript+="   xmlHttp.send(null);\n";
  javaScript+=" }\n";
  javaScript+=" setTimeout('process()',1000);\n";
  javaScript+="}\n";
  
  javaScript+="function handleServerResponse(){\n";
  javaScript+=" if(xmlHttp.readyState==4 && xmlHttp.status==200){\n";
    
  javaScript+="   xmlResponse=xmlHttp.responseXML;\n";
  javaScript+="   xmldoc = xmlResponse.getElementsByTagName('response');\n";
  //UPDATING VARIABLES ON PAGE
  javaScript+="   message = xmldoc[0];\n";
  javaScript+="   document.getElementById('var01').innerHTML = message.children[0].firstChild.data;\n";
  javaScript+="   document.getElementById('var02').innerHTML = message.children[1].firstChild.data;\n";
  javaScript+="   document.getElementById('var03').innerHTML = message.children[2].firstChild.data;\n";
  javaScript+="   document.getElementById('var04').innerHTML = message.children[3].firstChild.data;\n";
  javaScript+="   document.getElementById('var05').innerHTML = message.children[4].firstChild.data;\n";
  javaScript+="   document.getElementById('var06').innerHTML = message.children[5].firstChild.data;\n";
  javaScript+="   document.getElementById('var07').innerHTML = message.children[6].firstChild.data;\n";
  javaScript+="   document.getElementById('var08').innerHTML = message.children[7].firstChild.data;\n";
  javaScript+="   document.getElementById('var09').innerHTML = message.children[8].firstChild.data;\n";
  javaScript+="   document.getElementById('var10').innerHTML = message.children[9].firstChild.data;\n";
  javaScript+="   document.getElementById('var11').innerHTML = message.children[10].firstChild.data;\n";
  javaScript+="   document.getElementById('var12').innerHTML = message.children[11].firstChild.data;\n";
  javaScript+=" }\n";
  javaScript+="}\n";
  javaScript+="</SCRIPT>\n";
}

void buildXML(){
  XML="<?xml version='1.0'?>";
  XML+="<response>\n";
  XML+="  <rvar01>" + var01_lampState         + "</rvar01>\n";
  XML+="  <rvar02>" + var02_motionSensorState + "</rvar02>\n";
  XML+="  <rvar03>" + var03_buzzerState       + "</rvar03>\n";
  XML+="  <rvar04>" + var04_time              + "</rvar04>\n";
  XML+="  <rvar05>" + var05_date              + "</rvar05>\n";
  XML+="  <rvar06>" + var06_weekday           + "</rvar06>\n";
  XML+="  <rvar07>" + var07_alarmTime         + "</rvar07>\n";
  XML+="  <rvar08>" + var08_alarmDay          + "</rvar08>\n";
  XML+="  <rvar09>" + var09_alarmSet          + "</rvar09>\n";
  XML+="  <rvar10>" + var10_nightTimeStart    + "</rvar10>\n";
  XML+="  <rvar11>" + var11_nightTimeEnd      + "</rvar11>\n";
  XML+="  <rvar12>" + var12_nightTimeSet      + "</rvar12>\n";
  XML+="</response>\n";
}

