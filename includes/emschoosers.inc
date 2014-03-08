<?php
function notavail(){
  print("<td colspan=2 bgcolor=#777777><font size=-2 color=#cccccc><center>verfügbar nach 'Setzen'</center></font></td>");
}

function tempchooser($name,$min,$max,$step=1,$unit="°C",$empty="wählen",$sel=0,$specials=array()){
  print("<select  name=$name id=$name><option value=0>$empty</option>\n");
  for($i=$min;$i<=$max;$i+=$step){
    $dsc = "$i $unit";
    if (isset($specials[$i])) $dsc = $specials[$i];
    print("<option value='$i' ".($i==$sel?"selected":"").">$dsc</option>\n");
  }
  print("</select>");
}

function hourchooser($name,$sel=0){
  print("<select  name=$name id=$name>\n");
  for($i=0;$i<=23;$i++){
    $hs = substr("00".$i,-2).":00";
    print("<option value='$i' ".($i==$sel?"selected":"").">$hs</option>\n");
  }
  print("</select>");
}

function datechooser($name,$defv="TT.MM.JJJJ",$defb=""){
  $von="";
  if ($defb) $von="_von";
  print("<input type=text size=10 name=".$name.$von." id=".$name.$von." value=$defv >");
  if ($defb) print("<br> -  <input type=text size=10 name=".$name."_bis id=".$name."_bis value=$defb >\n");
}

function modechooser($name,$sel){
  print("<select  name=$name id=$name >");
  print("<option value=on ".($sel=="on"?"selected":"")." >ständig an</option>");
  print("<option value=off ".($sel=="off"?"selected":"")." >ständig aus</option>");
  print("<option value=auto ".($sel=="auto"?"selected":"")." >Automatik</option>");
  print("</select>");
}

function maintchooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=off ".($sel=="off"?"selected":"")." >keine</option>");
  print("<option value=bydate ".($sel=="bydate"?"selected":"")." >nach Datum</option>");
  print("<option value=byhours ".($sel=="byhours"?"selected":"")." >nach Betriebsstunden</option>");
  print("</select>");
}

function onoffchooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=on ".($sel=="on"?"selected":"")." >Aktiviert</option>");
  print("<option value=off ".($sel=="off"?"selected":"")." >Deaktiviert</option>");
  print("</select>");
}

function redmodechooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=offmode ".($sel=="offmode"?"selected":"")." >Abschaltbetrieb</option>");
  print("<option value=reduced ".($sel=="reduced"?"selected":"")." >Reduzierter Betrieb</option>");
  print("<option value=raumhalt ".($sel=="raumhalt"?"selected":"")." >Raumhaltbetrieb</option>");
  print("<option value=aussenhalt ".($sel=="aussenhalt"?"selected":"")." >Aussenhaltbetrieb</option>");
  print("</select>");
}

function refinputchooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=indoor ".($sel=="indoor"?"selected":"")." >Raumtemperatur</option>");
  print("<option value=outdoor ".($sel=="outdoor"?"selected":"")." >Aussentemperatur</option>");
  print("</select>");
}

function refinputvacchooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value='indoor' ".($sel=="indoor"?"selected":"").">Raumhaltbetrieb</option>\n");
  print("<option value='outdoor' ".($sel=="outdoor"?"selected":"").">Aussenhaltbetrieb</option>\n");
  print("</select>");
}


function frostmodechooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=off ".($sel=="off"?"selected":"")." >kein</option>");
  print("<option value=byindoortemp ".($sel=="byindoortemp"?"selected":"")." >per Raumtemperatur 5°C</option>");
  print("<option value=byoutdoortemp ".($sel=="byoutdoortemp"?"selected":"")." >per Aussentemperatur</option>");
  print("</select>");
}

function gebaeudechooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=light ".($sel=="light"?"selected":"")." >leicht</option>");
  print("<option value=medium ".($sel=="medium"?"selected":"")." >mittel</option>");
  print("<option value=heavy ".($sel=="heavy"?"selected":"")." >schwer</option>");
  print("</select>");
}


function daychooser($name,$sel){
  global $daystr2;
  $ds2 = $daystr2;
  $ds2["everyday"] = "täglich";
  print("<select  name=$name id=$name>");
  foreach ($ds2 as $k => $v){
    print("<option value=$k ".($sel==$k?"selected":"")." >$v</option>");
  }
  print("</select>");
}

function zirkchooser($name,$sel){
  $op = array("1x" => "1x 3min",
              "2x" => "2x 3min",  
              "3x" => "3x 3min",
              "4x" => "4x 3min",
              "5x" => "5x 3min",
              "6x" => "6x 3min",
              "alwayson" => "ständig");
              
  print("<select  name=$name id=$name>");
  foreach ($op as $k => $v){
    print("<option value=$k ".($sel==$k?"selected":"")." >$v</option>");
  }
  print("</select>");
}
?>