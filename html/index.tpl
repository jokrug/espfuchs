<html>
<head><title>EspFuchs</title>
<link rel="stylesheet" type="text/css" href="style.css"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no"/>

<script type="text/javascript">
  var ajax = null;
  
  if(window.XMLHttpRequest){ //Google Chrome, Mozilla Firefox, Opera, Safari, IE 7
    ajax = new XMLHttpRequest();
  }
  else if(window.ActiveXObject){ // Internet Explorer 6 und niedriger
    try{
      ajax = new ActiveXObject("Msxml2.XMLHTTP.6.0");
    } catch(e){
      try{
        ajax = new ActiveXObject("Msxml2.XMLHTTP.3.0");
      }
      catch(e){}
    }
  }
  if(ajax==null)
    alert("Ihr Browser unterstützt kein Ajax!");

  function process(){
    if((ajax!=null) && (ajax.readyState==0 || ajax.readyState==4)){
      ajax.open('POST','/xml',true);
      ajax.onreadystatechange=handleServerResponse;
      ajax.send(null);
    }
    setTimeout('process()',1000);
  }

  function handleServerResponse(){
    if(ajax.readyState==4 && ajax.status==200 && ajax.responseXML!=null){
      var currTime = ajax.responseXML.getElementsByTagName("curtime");
      document.getElementById('curtime').innerHTML=currTime[0].textContent;
      var battVb = ajax.responseXML.getElementsByTagName("battvb");
      document.getElementById('battvb').innerHTML=battVb[0].textContent;
    }
  }
</script>
</head>
<body onload="process()">
<div id="main">
<h2><img src="icons/ardf-online-logo.gif"> ESPFuchs %foxno%</h2>
<p>
<table id="menu"><tr>
    <td style="background-color: #f4e4ca">Info</a></td>
    <td><a href="foxhunt.html" style="display:block;margin:0px;">Fuchsjagd</a></td>
    <td><a href="testfuncs.html" style="display:block;margin:0px;">Test</a></td>
    <td><a href="settings.html" style="display:block;margin:0px;">Einstellung</a></td>
</table>
<hr>
<table>
<tr><td>Frequenz:        </td> <td><b>%freq%</b> KHz</td> </tr>
<tr><td>Modus:           </td> <td><b>%foxmode%</b></td> </tr>
<tr><td>Startzeit:       </td> <td><b>%starttime%</b></td> </tr>
<tr><td>Stopzeit:        </td> <td><b>%stoptime%</b></td> </tr>
<br/>
<tr><td>Aktuelle Uhrzeit:</td> <td><b id="curtime"></b></td> </tr>
<tr><td>Batteriespannung:</td> <td><b id="battvb"></b> V</td> </tr>
</table>
<hr>

<form action="/definefox">
  <table>
<tr><td><label>neue Frequenz:</label></td> <td>
    <select name="frequency_enum">%frequencyselectoptions%</select> kHz</td> </tr>
    <tr><td><label>neue Uhrzeit:</label></td> <td> <input type="time" name="setcurrtime"></td> </tr>
    <tr><td><label>neue Startzeit:</label></td> <td> <input type="time" name="setstarttime"></td> </tr>
    <tr><td><label>neue Stopzeit:</label></td> <td> <input type="time" name="setstoptime"></td> </tr>
  </table>
  <button type="submit" height=12px name="submit">Daten &uuml;bernehmen</button>
</form>

</p>
</div>
</body></html>
