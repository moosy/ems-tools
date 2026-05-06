<?php
function notavail(){
  print("<td colspan=2 bgcolor=#777777><font size=-2 color=#cccccc><center>verfügbar nach 'Setzen'</center></font></td>");
}

function
tempchooser($name,$min,$max,$step=1,$unit="°C",$empty="",$sel=0,$specials=array(),$updt=""){

  print("<div class='box'>");
  print("<select name=$name id=$name $updt>\n");
  if ($empty) print("<option value=0>$empty</option>\n");
  for($i=$min;$i<=$max;$i+=$step){
    $dsc = "$i $unit";
    if (isset($specials[$i])) $dsc = $specials[$i];
    print("<option value='$i' ".($i==$sel?"selected":"").">$dsc</option>\n");
  }
  print("</select></div>");
}

function threewaychooser($name,$sel,$updt=""){
  print("<div class='box'>");
  print("<select  name=$name id=$name $updt>");
  print("<option value=0 ".($sel=="0"?"selected":"")." >Heizen</option>");
  print("<option value=1 ".($sel=="1"?"selected":"")." >Warmwasser</option>");
  print("<option value=2 ".($sel=="2"?"selected":"")." >Mittelstellung</option>");
  print("</select></div>");
}

function ex15mchooser($name,$sel,$updt=""){
  print("<div class='box'>");
  print("<select  name=$name id=$name $updt>");

  print("<option value=1 ".($sel=="1"?"selected":"")." >15 min</option>");
  print("<option value=2 ".($sel=="2"?"selected":"")." >30 min</option>");
  print("<option value=3 ".($sel=="3"?"selected":"")." >45 min</option>");
  print("<option value=4 ".($sel=="4"?"selected":"")." >1 h</option>");
  print("<option value=5 ".($sel=="5"?"selected":"")." >1h 15min</option>");
  print("<option value=6 ".($sel=="6"?"selected":"")." >1h 30min</option>");
  print("<option value=7 ".($sel=="7"?"selected":"")." >1h 45min</option>");
  print("<option value=8 ".($sel=="8"?"selected":"")." >2 h</option>");
  print("<option value=9 ".($sel=="9"?"selected":"")." >2h 15min</option>");
  print("<option value=10 ".($sel=="10"?"selected":"")." >2h 30min</option>");
  print("<option value=11 ".($sel=="11"?"selected":"")." >2h 45min</option>");
  print("<option value=12 ".($sel=="12"?"selected":"")." >3 h</option>");
  print("<option value=13 ".($sel=="13"?"selected":"")." >3h 15min</option>");
  print("<option value=14 ".($sel=="14"?"selected":"")." >3h 30min</option>");
  print("<option value=15 ".($sel=="15"?"selected":"")." >3h 45min</option>");
  print("<option value=16 ".($sel=="16"?"selected":"")." >4 h</option>");

  print("</select></div>");
}


function hourchooser($name,$sel=0,$showinvalid=false){
  print("<div class='box'>");

  print("<select  name=$name id=$name>\n");
  for($i=0;$i<=23;$i++){
    for ($j=0; $j<4; $j++){
      $hs = substr("00".$i,-2).":".substr("00".($j*15),-2);
      $js = ($j*0.25)+$i;
      print("<option value='$js' ".($js==$sel?"selected":"").">$hs</option>\n");
    }
  }
  if ($showinvalid)  print("<option value='255' ".(255==$sel?"selected":"").">frei</option>\n");

  print("</select></div>");
}

function readdatechooser($name,$sel=0){
  print("<div class='box'>");

  print("<select  name=$name id=$name>\n");
  for ($j=0; $j<=6; $j++){
      $js = chr(65+$j);
      print("<option value='$j' ".($j==$sel?"selected":"").">Zeile $js</option>\n");
  }
  if ($showinvalid)  print("<option value='255' ".(255==$sel?"selected":"").">frei</option>\n");

  print("</select></div>");
}

function datechooser($name,$defv="TT.MM.JJJJ",$defb=""){
  $von="";
  if ($defb) $von="_von";
  print("<input type=text size=10 name=".$name.$von." id=".$name.$von." value=$defv >");
  if ($defb) print("<br> -  <input type=text size=10 name=".$name."_bis id=".$name."_bis value=$defb >\n");
}

function wwmodechooser($name,$sel){

#mode off|eco|comfort|followheater|auto

  print("<div class='box'>");

  print("<select  name=$name id=$name >");
  print("<option value=off ".($sel=="off"?"selected":"")." >aus</option>");
  print("<option value=eco ".($sel=="eco"?"selected":"")." >Eco</option>");
  print("<option value=comfort ".($sel=="comfort"?"selected":"")." >Komfort</option>");
  print("<option value=followheater ".($sel=="followheater"?"selected":"")." >wie Heizung</option>");
  print("<option value=auto ".($sel=="auto"?"selected":"")." >Automatik</option>");
  print("</select></div>");
}

function schwwmodechooser($name,$sel){

  print("<div class='box'>");
  print("<select  name=$name id=$name >");
  print("<option value=0 ".($sel=="W0"?"selected":"")." >aus</option>");
  print("<option value=1 ".($sel=="W1"?"selected":"")." >Eco</option>");
  print("<option value=2 ".($sel=="W2"?"selected":"")." >Komfort</option>");
  print("</select></div>");
}

function schhkmodechooser($name,$sel){

  print("<div class='box'>");
  print("<select  name=$name id=$name >");
  print("<option value=3 ".($sel=="H3"?"selected":"")." >Tag</option>");
  print("<option value=1 ".($sel=="H1"?"selected":"")." >Nacht</option>");
  print("</select></div>");
}


function schzpmodechooser($name,$sel){

  print("<div class='box'>");
  print("<select  name=$name id=$name >");
  print("<option value=0 ".($sel=="Z0"?"selected":"")." >Aus</option>");
  print("<option value=255 ".($sel=="Z255"?"selected":"")." >Ein</option>");
  print("</select></div>");
}



function hkmodechooser($name,$sel){

  print("<div class='box'>");

  print("<select  name=$name id=$name >");
  print("<option value=off ".($sel=="off"?"selected":"")." >aus</option>");
  print("<option value=manual ".($sel=="manual"?"selected":"")." >manuell</option>");
  print("<option value=auto ".($sel=="auto"?"selected":"")." >Automatik</option>");
  print("</select></div>");
}

function hksummermodechooser($name,$sel){

  print("<div class='box'>");

  print("<select  name=$name id=$name >");
  print("<option value=auto ".($sel=="auto"?"selected":"")." >Automatik</option>");
  print("<option value=heateron ".($sel=="heateron"?"selected":"")." >durchheizen</option>");
  print("</select></div>");
}


function zpmodechooser($name,$sel){

#zirkmode off|on|followww|auto

  print("<div class='box'>");

  print("<select  name=$name id=$name >");
  print("<option value=off ".($sel=="off"?"selected":"")." >Aus</option>");
  print("<option value=on ".($sel=="on"?"selected":"")." >st&auml;ndig an</option>");
  print("<option value=followww ".($sel=="followww"?"selected":"")." >wie WW</option>");
  print("<option value=auto ".($sel=="auto"?"selected":"")." >Automatik</option>");
  print("</select></div>");
}



function modechooser($name,$sel){
  print("<select  name=$name id=$name >");
  print("<option value=on ".($sel=="on"?"selected":"")." >ständig an</option>");
  print("<option value=off ".($sel=="off"?"selected":"")." >ständig aus</option>");
  print("<option value=auto ".($sel=="auto"?"selected":"")." >Automatik</option>");
  print("</select>");
}

function maintchooser($name,$sel){
  print("<div class='box'>");
  print("<select  name=$name id=$name>");
  print("<option value=off ".($sel=="off"?"selected":"")." >keine</option>");
  print("<option value=bydate ".($sel=="bydate"?"selected":"")." >nach Datum</option>");
  print("<option value=byhours ".($sel=="byhours"?"selected":"")." >nach Brennerstunden</option>");
  print("<option value=bymonths ".($sel=="bymonths"?"selected":"")." >nach Betriebsmonaten</option>");
  print("</select></div>");
}

function onoffchooser($name,$sel){
  print("<div class='box'>");
  print("<select  name=$name id=$name>");
  print("<option value=on ".($sel=="on"?"selected":"")." >Aktiviert</option>");
  print("<option value=off ".($sel=="off"?"selected":"")." >Deaktiviert</option>");
  print("</select></div>");
}

function onoffswitch($name,$sel,$updt=""){
  print("<label class=switch><input type=checkbox name=$name id=$name $updt");
  print($sel=="on"?"checked":"");
  print("><span class='slider round'></span></label>");
}

function redmodechooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=offmode ".($sel=="offmode"?"selected":"")." >Abschaltbetrieb</option>");
  print("<option value=reduced ".($sel=="reduced"?"selected":"")." >Reduzierter Betrieb</option>");
  print("<option value=raumhalt ".($sel=="raumhalt"?"selected":"")." >Raumhaltbetrieb</option>");
  print("<option value=aussenhalt ".($sel=="aussenhalt"?"selected":"")." >Aussenhaltbetrieb</option>");
  print("</select>");
}

function heatingsystemchooser($name,$sel){
  print("<select  name=$name id=$name>");
  print("<option value=none ".($sel=="none"?"selected":"")." >Keines</option>");
  print("<option value=heater ".($sel=="heater"?"selected":"")." >Heizk&ouml;rper</option>");
  print("<option value=convection ".($sel=="convection"?"selected":"")." >Konvektor</option>");
  print("<option value=floorheater ".($sel=="floorheater"?"selected":"")." >Fu&szlig;bodenheizung</option>");
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
  print("<div class='box'>");
  print("<select  name=$name id=$name>");
  print("<option value=light ".($sel=="light"?"selected":"")." >leicht</option>");
  print("<option value=medium ".($sel=="medium"?"selected":"")." >mittel</option>");
  print("<option value=heavy ".($sel=="heavy"?"selected":"")." >schwer</option>");
  print("</select></div>");
}


function daychooser($name,$sel){
  global $daystr2;
  $ds2 = $daystr2;
  $ds2["everyday"] = "täglich";

  print("<div class='box'>");

  print("<select  name=$name id=$name>");
  
  foreach ($ds2 as $k => $v){
    print("<option value=$k ".($sel==$k?"selected":"")." >$v</option>");
  }
  print("</select></div>");
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