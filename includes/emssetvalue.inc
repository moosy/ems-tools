<?php
require_once("/emsincludes/emsqry.inc");
if (isset($_GET["source"])){
  $key = trim($_GET["source"]);
  $val = trim($_GET["value"]);
  $_POST[$key] = $val;
  $_POST["set".$key] = "DOIT";
  if (($key == "holiday") || ($key == "vacation")){
    $vala = explode("-",$val);
    $_POST[$key."_von"] = $vala[0];
    $_POST[$key."_bis"] = $vala[1];

  }
}

function flipdate($s){
  if ($s=="") $s = "01.01.2000";
  $s = str_replace(".","-",trim($s));
  $sa = explode("-",$s);
  $day = substr("00".$sa[0],-2);
  $month = substr("00".$sa[1],-2);
  $year = "2".substr("000".$sa[2],-3);
  $res = ($year."-".$month."-".$day);
  return $res;
}

$sets = array("nachtred" => "hk1 reducedmodethreshold",
	      "urlaubnachtred" => "hk1 vacationreducedmodethreshold",
              "frost"    => "hk1 frostprotecttemperature",
              "summertime"  => "hk1 summerwinterthreshold",
              "absquit"  => "hk1 cancelreducedmodethreshold",
              "urlaubt"  => "hk1 vacationtemperature",
              "minvorlauf" => "hk1 minheatflowtemperature",
              "maxvorlauf" => "hk1 maxheatflowtemperature",
              "maxroomeffect"    => "hk1 maxroomeffect",
              "schedoptimizer"   => "hk1 scheduleoptimizer",
              "hkmode"    => "hk1 mode",
              "redmode"    => "hk1 reductionmode",
              "refinput"   => "hk1 relevantparameter",
              "refinputvac" => "hk1 vacationreductionmode",
              "frostmode" => "hk1 frostprotectmode",
              "limittemp" => "ww limittemperature",
              "loadled"   => "ww showloadindicator",
              "spzir"    => "ww zirkpump count",
              "zirmode"  => "ww zirkpump mode",
              "wwmode"   => "ww mode",
              "tdstat"   => "ww thermdesinfect mode",
              "tdtemp"   => "ww thermdesinfect temperature",
              "tdday"    => "ww thermdesinfect day",
              "tdhour"    => "ww thermdesinfect hour",
              "wwload"    => "ww loadonce",
              "wwstopload"  => "ww cancelload",
              "antipen"  => "uba antipendel",
              "contact1"  => "rc setcontactinfo 1",
              "contact2"  => "rc setcontactinfo 2",
              "kpnachl"  => "uba pumpdelay",
              "kpmod"    => "uba pumpmodulation",
              "maint"    => "uba schedulemaintenance",
              "vacation" => "hk1 vacationmode",
              "holiday"  => "hk1 holidaymode",
              "hystein"  => "uba hyst on",
              "hystaus"  => "uba hyst off",
              "wwtag"    => "ww temperature",
              "temptemp" => "hk1 temperatureoverride",
              "temptempoff" => "hk1 temperatureoverride off ",
              "day"      => "hk1 daytemperature",
              "night"    => "hk1 nighttemperature",
              "auslegtemp"  => "hk1 designtemperature",
              "gebaeude" => "rc buildingtype",
              "daempfung" => "rc outdoortempdamping",
              "minaussentemp" => "rc minoutdoortemperature",
              "raumoffset"  => "hk1 temperatureoffset",
              "party"    => "hk1 partymode",
              "pause"    => "hk1 pausemode");
              
foreach ($sets as $k => $v){
  if (isset($_POST["set".$k])){
      $nv = trim($_POST[$k]);
      switch($k){ // Sonderbehandlungen
        case "kpmod":
          $nv = str_replace("-"," ",$nv);
          break;      
 
        case "spzir":
          $nv = str_replace("x","",$nv);
          break;          

        case "holiday":
        case "vacation":
        
          $nv = flipdate(trim($_POST[$k."_von"]))." ".flipdate(trim($_POST[$k."_bis"]));
          $nv = str_replace(".","-",$nv);
          break;  

        case "maint":
          $nva = explode("-",$nv);
          if ($nva[0]=="bydate") $nva[1] = flipdate($nva[1]);
          $nv = implode (" ",$nva);
          break;

      }
      $emscommand = $v." ".$nv;
      doEmsCommand($emscommand);
  }
}
