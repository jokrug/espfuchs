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
<h1><img src="icons/ardf-online-logo.gif"> ESPFuchs 2</h1>
<p>
<hr>
<table><form>
  <tr><td>Aktuelle Uhrzeit:</td><td><div id="curtime"></div></td> </tr>
  <tr><td>Neue Uhrzeit:    </td><td><input type="time" name="settime"></td></tr>
  <tr><td></td> <td><button type="submit" height=15px name="submit">Daten &uuml;bernehmen</button></td></tr>
</form></table>
<li>Access counter: <b>%counter%</b>.</li>
</p>
</div>
</body></html>
